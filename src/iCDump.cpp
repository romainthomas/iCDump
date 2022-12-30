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
#include "iCDump/iCDump.hpp"
#include "iCDump/ObjC.hpp"
#include "log.hpp"
#include "MachOStream.hpp"

#include "LIEF/MachO.hpp"

using namespace LIEF::MachO;

namespace iCDump {

namespace ObjC {
std::unique_ptr<Metadata> parse(const std::string& file_path, ARCH arch) {
  static const ParserConfig PARSER_CONFIG = {
    .parse_dyld_exports = true, .parse_dyld_bindings = true, .parse_dyld_rebases = false
  };

  auto fat_bin = LIEF::MachO::Parser::parse(file_path, PARSER_CONFIG);
  if (!fat_bin || fat_bin->empty()) {
    ICDUMP_ERR("Can't parse {}", file_path);
    return nullptr;
  }
  // Look for the best architecture according to the arch parameter
  if (auto arm64_bin  = fat_bin->take(CPU_TYPES::CPU_TYPE_ARM64)) {
    return ObjC::Parser::parse(*arm64_bin);
  }

  if (auto x86_64_bin = fat_bin->take(CPU_TYPES::CPU_TYPE_X86_64)) {
    return ObjC::Parser::parse(*x86_64_bin);
  }

  ICDUMP_ERR("Can't find a supported architecture");
  return nullptr;
}
}
}
