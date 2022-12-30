/* Copyright 2023 R. Thomas
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "log.hpp"
#include "iCDump/config.hpp"
#include "iCDump/ObjC/Class.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/Protocol.hpp"
#include "iCDump/ObjC/Property.hpp"
#include "iCDump/ObjC/IVar.hpp"
#include <LIEF/BinaryStream/BinaryStream.hpp>
#include <LIEF/MachO/Binary.hpp>

#include "ClangAST/utils.hpp"

#ifdef ICDUMP_LLVM_SUPPORT
#include "swift/Demangling/Demangle.h"
#endif

namespace iCDump::ObjC {

Class::Class() = default;

std::unique_ptr<Class> Class::create(Parser& parser) {
  LIEF::BinaryStream& stream = parser.stream();
  const size_t P = stream.pos();
  auto cls_obj = stream.peek<ObjC::objc_class_t>();
  if (!cls_obj) {
    ICDUMP_ERR("Can't read objc_class_t at 0x{:x}", P);
    return nullptr;
  }

  // We assume that the classes metadata are class_ro_t
  uintptr_t cls_ro_ptr = parser.decode_ptr(cls_obj->bits.class_ro_ptr());
  auto raw_ro_cls = stream.peek<ObjC::class_ro_t>(cls_ro_ptr);
  if (!raw_ro_cls) {
    if (parser.bin().memory_base_address() > 0) {
      raw_ro_cls = stream.peek<ObjC::class_ro_t>(cls_obj->bits.class_ro_ptr2());
    }
  }

  if (!raw_ro_cls) {
    ICDUMP_ERR("Can't read class_ro_t at 0x{:x}", cls_obj->bits.class_ro_ptr());
    //ICDUMP_DEBUG("ro(): 0x{:010x}", cls_obj->bits.class_ro_ptr2());
    return nullptr;
  }

  std::string name;
  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_ro_cls->name))) {
    name = std::move(*res);
  } else {
    ICDUMP_ERR("Can't read class_ro_t.name at 0x{:x}", raw_ro_cls->name);
    return nullptr;
  }

  ICDUMP_DEBUG("Processing class {}", name);

  auto cls = std::make_unique<Class>();

  cls->name_           = name;
  cls->flags_          = raw_ro_cls->flags;
  cls->instance_start_ = raw_ro_cls->instance_start;
  cls->instance_size_  = raw_ro_cls->instance_size;
  cls->reserved_       = raw_ro_cls->reserved;
  // TODO(romain): Process the other fields (like weakIvarLayout)

  if (cls_obj->super_class && parser.decode_ptr(cls_obj->super_class) != stream.pos()) {
    const uintptr_t super_cls_ptr = parser.decode_ptr(cls_obj->super_class);
    ICDUMP_DEBUG("Parsing super class at 0x{:010x}", super_cls_ptr);
    LIEF::ScopedStream scoped_stream(stream, super_cls_ptr);
    if (std::unique_ptr<Class> super_cls = create(parser)) {
      for (auto&& m : super_cls->methods_) {
        m->is_instance_ = false;
        cls->methods_.push_back(std::move(m));
      }
    }
  }

  const bool is_meta = cls->is_meta();

  if (raw_ro_cls->base_method_list) {
    ICDUMP_DEBUG("  Class.base_method_list");
    LIEF::ScopedStream scoped_stream(stream, parser.decode_ptr(raw_ro_cls->base_method_list));
    if (const auto method_list = stream.read<ObjC::method_list_t>()) {
      const bool is_small = method_list->flags() & ObjC::method_list_t::IS_SMALL;
      const size_t methods_addr = stream.pos();
      const size_t sizeof_meth = is_small ? sizeof(ObjC::small_method_t) :
                                            sizeof(ObjC::big_method_t);

      for (size_t i = 0; i < method_list->count; ++i) {
        stream.setpos(methods_addr + i * sizeof_meth);
        ICDUMP_DEBUG("base_method_list[{}]@0x{:010x}", i, stream.pos());
        if (std::unique_ptr<Method> method = Method::create(parser, is_small)) {
          method->is_instance_ = !is_meta;
          cls->methods_.push_back(std::move(method));
        } else {
          ICDUMP_ERR("Error while processing base methods #{:d}", i);
          break;
        }
      }
    } else {
      ICDUMP_WARN("Can't read method_list_t@0x{:010x}", stream.pos());
    }
  }

  if (raw_ro_cls->base_protocols) {
    ICDUMP_DEBUG("  Class.base_protocols");
    LIEF::ScopedStream scoped_stream(stream, parser.decode_ptr(raw_ro_cls->base_protocols));
    if (const auto list = stream.read<ObjC::protocol_list_t>()) {
      for (size_t i = 0; i < list->count; ++i) {
        uintptr_t proto_addr = 0;
        if (auto res = stream.read<uintptr_t>()) {
          proto_addr = parser.decode_ptr(*res);
        } else {
          break;
        }

        if (Protocol* proto = parser.get_or_create_protocol(proto_addr)) {
          cls->protocols_.push_back(proto);
        } else {
          ICDUMP_ERR("Error while processing protocol #{:d}", i);
        }
      }
    }
  }

  if (raw_ro_cls->ivars) {
    ICDUMP_DEBUG("  Class.ivars");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_ro_cls->ivars));
    if (const auto list = stream.read<ObjC::ivars_list_t>()) {
      const size_t addr = stream.pos();
      for (size_t i = 0; i < list->count; ++i) {
        stream.setpos(addr + i * sizeof(ObjC::ivar_t));
        if (std::unique_ptr<IVar> ivar = IVar::create(parser)) {
          cls->ivars_.push_back(std::move(ivar));
        }
      }
    }
  }

  if (raw_ro_cls->base_properties) {
    ICDUMP_DEBUG("  Class.base_properties");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_ro_cls->base_properties));
    if (const auto prop_list = stream.read<ObjC::properties_list_t>()) {
      const size_t props_addr = stream.pos();
      for (size_t i = 0; i < prop_list->count; ++i) {
        stream.setpos(props_addr + i * sizeof(ObjC::property_t));
        if (std::unique_ptr<Property> prop = Property::create(parser)) {
          cls->properties_.push_back(std::move(prop));
        }
      }
    }
  }

  return cls;
}

std::string Class::to_string() const {
  return "";
}

std::string Class::to_decl() const {
  if constexpr (icdump_llvm_support) {
    return ClangAST::generate(*this);
  } else {
    return "";
  }
}


std::string Class::demangled_name() const {
#ifdef ICDUMP_LLVM_SUPPORT
    return swift::Demangle::demangleSymbolAsString(name_);
#else
    return name_;
#endif
}

}
