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

#include "LIEF/MachO.hpp"
#include "MachOStream.hpp"
#include "log.hpp"

using namespace LIEF::MachO;
namespace iCDump {

MachOStream::MachOStream(const LIEF::MachO::Binary& bin) :
  binary_{&bin}
{}

uint64_t MachOStream::size() const {
  // TODO(romain): To fix
  return static_cast<uint64_t>(-1);
}

LIEF::result<const void*> MachOStream::read_at(uint64_t offset, uint64_t size) const {
  uint64_t r_offset = offset;
  if (binary_->memory_base_address() > 0 && offset > binary_->memory_base_address()) {
    ICDUMP_DEBUG("  0x{:010x} -> 0x{:010x})", offset, offset - binary_->memory_base_address() + binary_->imagebase());
    r_offset -= binary_->memory_base_address();
    r_offset += binary_->imagebase();
  }
  const SegmentCommand* seg = binary_->segment_from_virtual_address(r_offset);
  if (seg == nullptr) {
    ICDUMP_DEBUG("Can't find segment with offset: 0x{:010x}", r_offset);
    return make_error_code(lief_errors::read_error);
  }
  LIEF::span<const uint8_t> content = seg->content();
  uintptr_t delta = r_offset - seg->virtual_address();
  return content.data() + delta;
}
}
