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
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/Metadata.hpp"
#include "iCDump/ObjC/Class.hpp"
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Protocol.hpp"
#include "iCDump/ObjC/Property.hpp"
#include "iCDump/ObjC/IVar.hpp"
#include "MachOStream.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "log.hpp"

#include "LIEF/MachO.hpp"
#include "LIEF/BinaryStream/SpanStream.hpp"

using namespace LIEF::MachO;

namespace iCDump::ObjC {


const Section* get_objc_section(const Binary& bin, const std::string& name) {
  if (const auto* sec = bin.get_section("__DATA", name)) {
    return sec;
  }

  if (const auto* sec = bin.get_section("__DATA_CONST", name)) {
    return sec;
  }

  if (const auto* sec = bin.get_section("__DATA_DIRTY", name)) {
    return sec;
  }
  ICDUMP_DEBUG("Can't find '{}'", name);
  return nullptr;
}

inline const Section* get_objc_classlist(const Binary& bin) {
  return get_objc_section(bin, "__objc_classlist");
}

inline const Section* get_objc_protolist(const Binary& bin) {
  return get_objc_section(bin, "__objc_protolist");
}

Parser::Parser() = default;

Parser::Parser(const Binary* bin) :
  bin_{bin},
  stream_{std::make_unique<MachOStream>(*bin)},
  metadata_{std::make_unique<Metadata>()},
  imagebase_{bin_->imagebase()}
{
}

std::unique_ptr<Metadata> Parser::parse(const Binary& bin) {
  Parser parser(&bin);

  parser
    .process_protocols()
    .process_classes();

  return std::move(parser.metadata_);
}


Protocol* Parser::get_or_create_protocol(uintptr_t offset) {
  if (auto it = protocols_.find(offset); it != std::end(protocols_)) {
    return it->second;
  }


  LIEF::BinaryStream& mstream = stream();
  LIEF::ScopedStream scoped(mstream, offset);

  std::unique_ptr<Protocol> proto = Protocol::create(*this);
  if (!proto) {
    ICDUMP_ERR("Error while parsing protocol at offset 0x{:x}", offset);
    return nullptr;
  }
  auto* raw_ptr = proto.get();
  metadata_->protocol_lookup_[proto->mangled_name()] = proto.get();
  metadata_->protocols_.push_back(std::move(proto));

  return raw_ptr;
}

Parser& Parser::process_protocols() {
  if (const Section* sec = get_objc_protolist(*bin_)) {
    ICDUMP_DEBUG("ObjC Protocol from: {}: 0x{:010x}", sec->name(), sec->virtual_address());
    LIEF::SpanStream protolist(sec->content());
    return process_protocols(*stream_, protolist);
  }
  return *this;
}

Parser& Parser::process_protocols(LIEF::BinaryStream& mstream, LIEF::BinaryStream& protolist) {
  const size_t nb_protos = protolist.size() / sizeof(uintptr_t);
  ICDUMP_DEBUG("Nb protocols: {:d}", nb_protos);
  for (size_t i = 0; i < nb_protos; ++i) {
    uintptr_t location = 0;
    ICDUMP_DEBUG("  __objc_protolist[{}]", i);
    if (auto res = protolist.read<uintptr_t>()) {
      location = decode_ptr(*res);
    } else {
      ICDUMP_WARN("Can't read __objc_protolist[{}]", i);
      break;
    }
    {
      ICDUMP_DEBUG("  __objc_protolist@0x{:010x}", location);
      LIEF::ScopedStream scoped(mstream, location);
      if (std::unique_ptr<Protocol> proto = Protocol::create(*this)) {
        protocols_[location] = proto.get();

        metadata_->protocol_lookup_[proto->mangled_name()] = proto.get();
        metadata_->protocols_.push_back(std::move(proto));
      } else {
        ICDUMP_WARN("Can't read __objc_protolist@0x{:010x}", location);
      }
    }
  }
  return *this;
}

Parser& Parser::process_classes() {
  if (const Section* sec = get_objc_classlist(*bin_)) {
    LIEF::SpanStream classlist(sec->content());
    return process_classes(*stream_, classlist);
  }
  return *this;
}

Parser& Parser::process_classes(LIEF::BinaryStream& mstream, LIEF::BinaryStream& classlist) {
  const size_t nb_classes = classlist.size() / sizeof(uintptr_t);

  ICDUMP_DEBUG("__objc_classlist: #{}", nb_classes);
  for (size_t i = 0; i < nb_classes; ++i) {
    uintptr_t location = 0;
    if (auto res = classlist.read<uintptr_t>()) {
      ICDUMP_DEBUG("  __objc_classlist[{}]: 0x{:010x}", i, location);
      location = decode_ptr(*res);
    } else {
      ICDUMP_WARN("Can't read __objc_classlist[{}]", i);
      break;
    }
    {
      LIEF::ScopedStream scoped(mstream, location);
      ICDUMP_DEBUG("  __objc_classlist@{:010x}", location);
      if (std::unique_ptr<Class> cls = Class::create(*this)) {
        metadata_->classes_lookup_[cls->name()] = cls.get();
        metadata_->classes_.push_back(std::move(cls));
      } else {
        ICDUMP_WARN("Can't read __objc_classlist@0x{:010x}", location);
      }
    }
  }
  return *this;
}

uintptr_t Parser::decode_ptr(uintptr_t ptr) {
  uintptr_t decoded = ptr & ((1llu << 51) - 1);
  if (imagebase_ > 0 && decoded < imagebase_) {
    decoded += imagebase_;
  }
  //ICDUMP_DEBUG("DECODE(0x{:010x}): 0x{:010x}", ptr, decoded);
  return decoded;
}

}
