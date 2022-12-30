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
#ifndef ICDUMP_OBJC_PROTOCOL_H_
#define ICDUMP_OBJC_PROTOCOL_H_
#include <string>
#include <vector>
#include <memory>

#include "iCDump/iterators.hpp"

namespace iCDump::ObjC {
class Method;
class Parser;
class Property;

//! Mirror of protcol_t
class Protocol {
  public:
  friend class Parser;
  using methods_t    = std::vector<std::unique_ptr<Method>>;
  using properties_t = std::vector<std::unique_ptr<Property>>;

  using methods_it_t    = const_ref_iterator<const methods_t&, Method*>;
  using properties_it_t = const_ref_iterator<const properties_t&, Property*>;

  static std::unique_ptr<Protocol> create(Parser& parser);

  Protocol();
  Protocol(const Protocol&) = delete;
  Protocol& operator=(const Protocol&) = delete;

  inline const std::string& mangled_name() const {
    return mangled_name_;
  }

  inline methods_it_t optional_methods() const {
    return opt_methods_;
  }

  inline methods_it_t required_methods() const {
    return required_methods_;
  }

  inline properties_it_t properties() const {
    return properties_;
  }

  // TODO: demangled version
  //std::string name() const;

  std::string to_string() const;
  std::string to_decl() const;
  private:
  std::string mangled_name_;

  methods_t opt_methods_;
  methods_t required_methods_;
  properties_t properties_;
};

}
#endif
