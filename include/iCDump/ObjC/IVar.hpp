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
#ifndef ICDUMP_IVAR_H_
#define ICDUMP_IVAR_H_
#include <string>
#include <memory>

namespace iCDump::ObjC {
class Parser;
struct Type;

class IVar {
  public:
  friend class Parser;

  IVar() = default;
  static std::unique_ptr<IVar> create(Parser& parser);

  inline const std::string& name() const {
    return name_;
  }

  inline const std::string& mangled_type() const {
    return mangled_type_;
  }

  std::unique_ptr<Type> type() const;

  std::string to_string() const;
  std::string to_decl() const;

  private:
  std::string name_;
  std::string mangled_type_;
};

}
#endif
