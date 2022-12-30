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
#include "iCDump/ObjC/Metadata.hpp"
#include "iCDump/ObjC/Class.hpp"
#include "iCDump/ObjC/Protocol.hpp"

namespace iCDump::ObjC {
const Class* Metadata::get_class(const std::string& name) const {
  if (auto it = classes_lookup_.find(name); it != std::end(classes_lookup_)) {
    return it->second;
  }
  return nullptr;
}

const Protocol* Metadata::get_protocol(const std::string& name) const {
  if (auto it = protocol_lookup_.find(name); it != std::end(protocol_lookup_)) {
    return it->second;
  }
  return nullptr;
}

std::string Metadata::to_decl() const {
  std::string out;

  // First declare the protocols
  for (const auto& protocol : protocols_) {
    out += protocol->to_decl();
  }

  for (const auto& cls : classes_) {
    out += cls->to_decl();
  }

  return out;
}

std::string Metadata::to_string() const {
  return "";
}

}

