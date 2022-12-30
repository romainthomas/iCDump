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
#ifndef ICDUMP_OBJCMETHOD_H_
#define ICDUMP_OBJCMETHOD_H_
#include <string>
#include <vector>
#include <memory>

namespace iCDump::ObjC {
class Parser;
class Protocol;
class Class;
struct Type;

class Method {
  public:
  friend class Parser;
  friend class Protocol;
  friend class Class;

  struct prototype_t {
    std::unique_ptr<Type> rtype;
    std::vector<std::unique_ptr<Type>> params;
  };

  Method() = default;
  static std::unique_ptr<Method> create(Parser& parser, bool is_small);

  inline const std::string& name() const {
    return name_;
  }

  inline const std::string& mangled_type() const {
    return mangled_type_;
  }

  inline uintptr_t address() const {
    return addr_;
  }

  inline bool is_instance() const {
    return is_instance_;
  }

  prototype_t prototype() const;

  std::string to_string() const;

  private:
  std::string name_;
  std::string mangled_type_;
  uintptr_t   addr_ = 0;

  bool is_instance_ = true;
};

}
#endif
