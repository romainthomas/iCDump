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
#ifndef ICDUMP_OBJCCLASS_H_
#define ICDUMP_OBJCCLASS_H_
#include <string>
#include <vector>
#include <memory>

#include "iCDump/iterators.hpp"

namespace LIEF {
class BinaryStream;
}

namespace iCDump::ObjC {
class Protocol;
class Method;
class Parser;
class Property;
class IVar;

//! Mirror of class_ro_t
class Class {
  public:
  static constexpr auto META                       = 1 << 0;
  static constexpr auto ROOT                       = 1 << 1;
  static constexpr auto HAS_CXX_STRUCTORS          = 1 << 2;
  static constexpr auto HIDDEN                     = 1 << 4;
  static constexpr auto EXCEPTION                  = 1 << 5;
  static constexpr auto HAS_SWIFT_INITIALIZER      = 1 << 6;
  static constexpr auto IS_ARC                     = 1 << 7;
  static constexpr auto IS_HAS_CXX_DTOR_ONLY       = 1 << 8;
  static constexpr auto IS_HAS_WEAK_WITHOUT_ARC    = 1 << 9;
  static constexpr auto FORBIDS_ASSOCIATED_OBJECTS = 1 << 10;

  using protocols_t     = std::vector<Protocol*>;
  using methods_t       = std::vector<std::unique_ptr<Method>>;
  using properties_t    = std::vector<std::unique_ptr<Property>>;
  using ivars_t         = std::vector<std::unique_ptr<IVar>>;

  using methods_it_t    = const_ref_iterator<const methods_t&, Method*>;
  using ivars_it_t      = const_ref_iterator<const ivars_t&, IVar*>;
  using protocols_it_t  = const_ref_iterator<const protocols_t&>;
  using properties_it_t = const_ref_iterator<const properties_t&, Property*>;

  Class();
  Class(const Class&) = delete;
  Class& operator=(const Class&) = delete;

  static std::unique_ptr<Class> create(Parser& parser);

  inline const std::string& name() const {
    return name_;
  }

  inline const Class* super_class() const {
    return super_.get();
  }

  std::string demangled_name() const;

  inline bool is_meta() const {
    return flags_ & META;
  }

  inline methods_it_t methods() const { return methods_; }
  inline protocols_it_t protocols() const { return protocols_; }
  inline properties_it_t properties() const { return properties_; }
  inline ivars_it_t ivars() const { return ivars_; }

  std::string to_string() const;
  std::string to_decl() const;

  private:
  std::unique_ptr<Class> super_;

  uint32_t    flags_ = 0;
  std::string name_;
  uint32_t    instance_start_ = 0;
  uint32_t    instance_size_ = 0;
  uint32_t    reserved_ = 0;

  protocols_t  protocols_;
  methods_t    methods_;
  ivars_t      ivars_;
  properties_t properties_;
};

}
#endif
