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
#include "iCDump/ObjC/Property.hpp"
#include "iCDump/ObjC/Parser.hpp"
#include "iCDump/ObjC/Types.hpp"
#include "iCDump/config.hpp"
#include "log.hpp"

#include "ClangAST/utils.hpp"

#include <LIEF/BinaryStream/BinaryStream.hpp>

namespace iCDump::ObjC {
std::unique_ptr<Property> Property::create(Parser& parser) {
  LIEF::BinaryStream& stream = parser.stream();
  const auto raw_prop = stream.peek<ObjC::property_t>();
  if (!raw_prop) {
    return nullptr;
  }

  auto prop = std::make_unique<Property>();

  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_prop->name))) {
    prop->name_ = std::move(*res);
  }

  if (auto res = stream.peek_string_at(parser.decode_ptr(raw_prop->attributes))) {
    prop->attributes_ = std::move(*res);
  }
  return prop;
}


std::string Property::to_string() const {
  return "";
}

std::string Property::to_decl() const {
  if constexpr (icdump_llvm_support) {
    return ClangAST::generate(*this);
  } else {
    return "<LLVM NOT PROVIDED>";
  }
}
}
