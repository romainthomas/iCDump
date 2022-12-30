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
#include <LIEF/BinaryStream/BinaryStream.hpp>

#include "iCDump/ObjC/Protocol.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Property.hpp"
#include "iCDump/config.hpp"

#include "log.hpp"

#include "ClangAST/utils.hpp"

namespace iCDump::ObjC {
Protocol::Protocol() = default;

std::unique_ptr<Protocol> Protocol::create(Parser& parser) {
  LIEF::BinaryStream& stream = parser.stream();
  const auto raw_proto = stream.peek<ObjC::protocol_t>();
  if (!raw_proto) {
    return nullptr;
  }

  auto protocol = std::make_unique<Protocol>();

  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_proto->mangled_name))) {
    protocol->mangled_name_ = std::move(*res);
  }

  if (raw_proto->protocols) {
    // TODO(romain): To be processed
  }

  if (raw_proto->instance_methods) {
    ICDUMP_DEBUG("[->] protocol.instance_methods");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_proto->instance_methods));
    if (const auto method_list = stream.read<ObjC::method_list_t>()) {
      const bool is_small = method_list->flags() & ObjC::method_list_t::IS_SMALL;
      const size_t sizeof_meth = is_small ? sizeof(ObjC::small_method_t) : sizeof(ObjC::big_method_t);
      const size_t methods_addr = stream.pos();
      for (size_t i = 0; i < method_list->count; ++i) {
        stream.setpos(methods_addr + i * sizeof_meth);
        if (std::unique_ptr<Method> method = Method::create(parser, is_small)) {
          method->is_instance_ = true;
          protocol->required_methods_.push_back(std::move(method));
        }
      }
    } else {
      ICDUMP_ERR("Methods list seems corrupted");
    }
  }

  if (raw_proto->class_methods) {
    ICDUMP_DEBUG("[->] protocol.class_methods");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_proto->class_methods));
    if (const auto method_list = stream.read<ObjC::method_list_t>()) {
      const size_t methods_addr = stream.pos();
      const bool is_small = method_list->flags() & ObjC::method_list_t::IS_SMALL;
      const size_t sizeof_meth = is_small ? sizeof(ObjC::small_method_t) : sizeof(ObjC::big_method_t);
      for (size_t i = 0; i < method_list->count; ++i) {
        stream.setpos(methods_addr + i * sizeof_meth);
        if (std::unique_ptr<Method> method = Method::create(parser, is_small)) {
          method->is_instance_ = false;
          protocol->required_methods_.push_back(std::move(method));
        }
      }
    } else {
      ICDUMP_ERR("Methods list seems corrupted");
    }
  }

  if (raw_proto->optional_instance_methods) {
    ICDUMP_DEBUG("[->] protocol.optional_instance_methods");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_proto->optional_instance_methods));
    if (const auto method_list = stream.read<ObjC::method_list_t>()) {
      const bool is_small = method_list->flags() & ObjC::method_list_t::IS_SMALL;
      const size_t methods_addr = stream.pos();
      const size_t sizeof_meth = is_small ? sizeof(ObjC::small_method_t) : sizeof(ObjC::big_method_t);
      ICDUMP_DEBUG("     Count: {}", method_list->count);
      for (size_t i = 0; i < method_list->count; ++i) {
        ICDUMP_DEBUG("     protocol.optional_instance_methods[{}]", i);
        stream.setpos(methods_addr + i * sizeof_meth);
        if (std::unique_ptr<Method> method = Method::create(parser, is_small)) {
          method->is_instance_ = true;
          protocol->opt_methods_.push_back(std::move(method));
        }
      }
    } else {
      ICDUMP_ERR("Methods list seems corrupted");
    }
  }

  if (raw_proto->optional_class_methods) {
    ICDUMP_DEBUG("[->] protocol.optional_class_methods");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_proto->optional_class_methods));
    if (const auto method_list = stream.read<ObjC::method_list_t>()) {
      const bool is_small = method_list->flags() & ObjC::method_list_t::IS_SMALL;
      const size_t sizeof_meth = is_small ? sizeof(ObjC::small_method_t) : sizeof(ObjC::big_method_t);
      const size_t methods_addr = stream.pos();
      for (size_t i = 0; i < method_list->count; ++i) {
        stream.setpos(methods_addr + i * sizeof_meth);
        if (std::unique_ptr<Method> method = Method::create(parser, is_small)) {
          method->is_instance_ = false;
          protocol->opt_methods_.push_back(std::move(method));
        }
      }
    } else {
      ICDUMP_ERR("Methods list seems corrupted");
    }
  }


  if (raw_proto->instance_properties) {
    ICDUMP_DEBUG("[->] protocol.instance_properties");
    LIEF::ScopedStream scoped(stream, parser.decode_ptr(raw_proto->instance_properties));
    if (const auto prop_list = stream.read<ObjC::properties_list_t>()) {
      const size_t props_addr = stream.pos();
      for (size_t i = 0; i < prop_list->count; ++i) {
        stream.setpos(props_addr + i * sizeof(ObjC::property_t));
        if (std::unique_ptr<Property> prop = Property::create(parser)) {
          protocol->properties_.push_back(std::move(prop));
        }
      }
    }
  }
  return protocol;
}

std::string Protocol::to_string() const {
  std::string out;
  out += fmt::format("Mangled Name: {}\n", mangled_name_);
  return out;
}


std::string Protocol::to_decl() const {
  if constexpr (icdump_llvm_support) {
    return ClangAST::generate(*this);
  } else {
    return "<LLVM NOT PROVIDED>";
  }
}


}
