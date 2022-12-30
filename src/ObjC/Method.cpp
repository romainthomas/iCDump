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
#include "iCDump/ObjC/Method.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/TypesEncoding.hpp"
#include "log.hpp"

#include <LIEF/BinaryStream/BinaryStream.hpp>

namespace iCDump::ObjC {
std::unique_ptr<Method> Method::create(Parser& parser, bool is_small) {
  LIEF::BinaryStream& stream = parser.stream();
  ICDUMP_DEBUG("Parsing ObjC Method @0x{:x}", stream.pos());
  auto method = std::make_unique<Method>();
  if (is_small) {
    ICDUMP_DEBUG("meth@0x{:x}: is small", stream.pos());
    const size_t pos = stream.pos();
    const auto raw_method = stream.peek<ObjC::small_method_t>();
    if (!raw_method) {
      ICDUMP_WARN("meth@0x{:x}: can't read small_method_t", stream.pos());
      return nullptr;
    }

    if (auto str_ptr = stream.peek<uintptr_t>(pos + raw_method->name)) {
      const uintptr_t decoded = parser.decode_ptr(*str_ptr);
      if (auto res = stream.peek_string_at(decoded)) {
        method->name_ = std::move(*res);
      } else {
        ICDUMP_WARN("meth@0x{:x}: can't read name", stream.pos());
      }
    } else {
      ICDUMP_WARN("Can't read small method name ptr (0x{:010x})", pos + raw_method->name);
    }

    if (auto res = stream.peek_string_at(pos + offsetof(ObjC::small_method_t, types) + raw_method->types)) {
      method->mangled_type_ = std::move(*res);
    }
    method->addr_ = raw_method->imp;
    return method;
  }

  ICDUMP_DEBUG("meth@0x{:x}: is not small", stream.pos());
  auto raw_method = stream.peek<ObjC::big_method_t>();
  if (!raw_method) {
    ICDUMP_WARN("meth@0x{:x}: can't read big_method_t", stream.pos());
    return nullptr;
  }

  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_method->name))) {
    method->name_ = std::move(*res);
  }
  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_method->types))) {
    method->mangled_type_ = std::move(*res);
  }

  method->addr_ = raw_method->imp;

  return method;

}

Method::prototype_t Method::prototype() const {
  types_t types = decode_type(mangled_type());

  if (types.empty()) {
    ICDUMP_ERR("Decoding {} failed", mangled_type());
    return {};
  }
  // First extract the return type
  auto it = std::begin(types);
  std::unique_ptr<Type> ret = std::move(*it++);

  // Then the parameter types
  types_t params;
  for (; it != std::end(types); ++it) {
    params.push_back(std::move(*it));
  }
  types.clear();
  return {std::move(ret), std::move(params)};
}


std::string Method::to_string() const {
  std::string out;

  out += fmt::format("Name: {}\n", name_);
  out += fmt::format("Type: {}\n", mangled_type_);
  out += fmt::format("Addr: 0x{:x}", addr_);
  return out;
}
}
