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
#include "iCDump/ObjC/IVar.hpp"
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "iCDump/ObjC/TypesEncoding.hpp"
#include "log.hpp"

#include <LIEF/BinaryStream/BinaryStream.hpp>

namespace iCDump::ObjC {

std::unique_ptr<IVar> IVar::create(Parser& parser) {
  LIEF::BinaryStream& stream = parser.stream();
  const auto raw_ivar = stream.peek<ObjC::ivar_t>();

  if (!raw_ivar) {
    return nullptr;
  }

  auto ivar = std::make_unique<IVar>();
  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_ivar->name))) {
    ivar->name_ = std::move(*res);
  } else {
    ICDUMP_ERR("Can't read ivar.name at 0x{:x}", parser.decode_ptr(raw_ivar->name));
  }

  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_ivar->type))) {
    ivar->mangled_type_ = std::move(*res);
  } else {
    ICDUMP_ERR("Can't read ivar.type at 0x{:x}", parser.decode_ptr(raw_ivar->type));
  }

  return ivar;
}


std::unique_ptr<Type> IVar::type() const {
  if (mangled_type_.empty()) {
    return nullptr;
  }
  std::vector<std::unique_ptr<Type>> types = decode_type(mangled_type());
  if (types.size() != 1) {
    ICDUMP_ERR("Error while parsing type: {}", mangled_type());
    return nullptr;
  }
  return std::move(types.back());
}


std::string IVar::to_string() const {
  return fmt::format("{} {}", mangled_type(), name());
}

std::string IVar::to_decl() const {
  return "";
}
}
