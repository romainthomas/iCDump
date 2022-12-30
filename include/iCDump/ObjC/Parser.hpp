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
#ifndef ICDUMP_OBJC_PARSER_H_
#define ICDUMP_OBJC_PARSER_H_
#include <memory>
#include <unordered_map>
#include "iCDump/NonCopyable.hpp"
namespace LIEF {
class BinaryStream;
namespace MachO {
class Binary;
}
}

namespace iCDump::ObjC {
class Metadata;
class Class;
class Method;
class Protocol;
class Parser : protected NonCopyable {
  public:
  static std::unique_ptr<Metadata> parse(const LIEF::MachO::Binary& bin);

  inline LIEF::BinaryStream& stream() {
    return *stream_;
  }

  inline const LIEF::MachO::Binary& bin() const {
    return *bin_;
  }

  inline uintptr_t imagebase() const {
    return imagebase_;
  }

  Protocol* get_or_create_protocol(uintptr_t offset);

  uintptr_t decode_ptr(uintptr_t ptr);

  private:
  Parser& process_classes();
  Parser& process_classes(LIEF::BinaryStream& mstream, LIEF::BinaryStream& classlist);
  Parser& process_protocols();
  Parser& process_protocols(LIEF::BinaryStream& mstream, LIEF::BinaryStream& protolist);


  Parser();
  Parser(const LIEF::MachO::Binary* bin);
  const LIEF::MachO::Binary* bin_ = nullptr;
  uintptr_t imagebase_ = 0;
  std::unique_ptr<LIEF::BinaryStream>  stream_;
  std::unique_ptr<Metadata> metadata_;

  std::unordered_map<uintptr_t, Protocol*> protocols_;
};

}
#endif
