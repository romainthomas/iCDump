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
#ifndef ICDUMP_OBJC_METADATA_H_
#define ICDUMP_OBJC_METADATA_H_
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "iCDump/iterators.hpp"

namespace iCDump::ObjC {

// Forward definitions
class Class;
class Parser;
class Protocol;

class Metadata {
  friend class Parser;
  public:
  Metadata() = default;
  ~Metadata() = default;

  Metadata(const Metadata&) = delete;
  Metadata& operator=(const Metadata&) = delete;

  using classes_t   = std::vector<std::unique_ptr<Class>>;
  using protocols_t = std::vector<std::unique_ptr<Protocol>>;

  using classes_it_t  = const_ref_iterator<const classes_t&, Class*>;
  using protocol_it_t = const_ref_iterator<const protocols_t&, Protocol*>;

  inline classes_it_t classes() const {
    return classes_;
  }

  inline protocol_it_t protocols() const {
    return protocols_;
  }

  const Class* get_class(const std::string& name) const;
  const Protocol* get_protocol(const std::string& name) const;

  std::string to_decl() const;
  std::string to_string() const;

  private:
  classes_t classes_;
  std::unordered_map<std::string, Class*> classes_lookup_;

  protocols_t protocols_;
  std::unordered_map<std::string, Protocol*> protocol_lookup_;

};

}
#endif
