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
#ifndef ICDUMP_OBJCTYPES_H_
#define ICDUMP_OBJCTYPES_H_
#include <unistd.h>
#include <inttypes.h>
#include <type_traits>

namespace iCDump::ObjC {
struct objc_class_t;

using protocol_ref_t = uintptr_t;
using ptr_t          = uintptr_t;

struct objc_image_info_t {
  uint32_t version;
  uint32_t flags;
};

struct objc_object_t {
};


// Adaptation of PointerUnion defined in PointerUnion.h
template <class T1, class T2>
class PointerUnion {
  uintptr_t value_;

  static_assert(alignof(T1) >= 2, "alignment requirement");
  static_assert(alignof(T2) >= 2, "alignment requirement");

  struct IsPT1 {
    static const uintptr_t Num = 0;
  };
  struct IsPT2 {
    static const uintptr_t Num = 1;
  };

  uintptr_t pointer() const {
    return value_ & ~1;
  }

  uintptr_t tag() const {
    return value_ & 1;
  }

  public:
  template <typename T>
  inline bool is() const {
    if constexpr (std::is_same_v<T, T1>) {
      return tag() == IsPT1::Num;
    }

    if constexpr (std::is_same_v<T, T2>) {
      return tag() == IsPT2::Num;
    }
    return false;
  }
  template <typename T>
  inline T get() const {
    return reinterpret_cast<T>(pointer());
  }
};

struct method_list_t {
  static constexpr uint32_t FLAG_MASK = 0xffff0003;
  static constexpr uint32_t IS_SMALL  = 0x80000000;
  uint32_t entsize_and_flags;
  uint32_t count;

  inline uint32_t entsize() const {
    return entsize_and_flags & ~FLAG_MASK;
  }

  inline uint32_t flags() const {
    return entsize_and_flags & FLAG_MASK;
  }
};

struct property_t {
  ptr_t name;
  ptr_t attributes;
};

struct properties_list_t {
  static constexpr uint32_t FLAG_MASK = 0;
  uint32_t entsize_and_flags;
  uint32_t count;

  inline uint32_t entsize() {
    return entsize_and_flags & ~FLAG_MASK;
  }

  inline uint32_t flags() {
    return entsize_and_flags & FLAG_MASK;
  }
};


struct ivar_t {
  ptr_t offset;
  ptr_t name;
  ptr_t type;

  uint32_t alignment_raw;
  uint32_t size;
};

struct ivars_list_t {
  static constexpr uint32_t FLAG_MASK = 0;
  uint32_t entsize_and_flags;
  uint32_t count;

  inline uint32_t entsize() {
    return entsize_and_flags & ~FLAG_MASK;
  }

  inline uint32_t flags() {
    return entsize_and_flags & FLAG_MASK;
  }
};


// Listed in __objc_protolist
struct protocol_t {
  ptr_t    isa; // I don't know why :'( since objc_object should be empty ...
  ptr_t    mangled_name;
  ptr_t    protocols;
  ptr_t    instance_methods;
  ptr_t    class_methods;
  ptr_t    optional_instance_methods;
  ptr_t    optional_class_methods;
  ptr_t    instance_properties;
  uint32_t size;
  uint32_t flags;


};


struct protocol_list_t {
  uintptr_t count;
};


struct class_ro_t {
  uint32_t flags;
  uint32_t instance_start;
  uint32_t instance_size;
  uint32_t reserved; // Only for x64
  union {
    ptr_t ivar_layout;
    ptr_t non_metaclass;
  };
  ptr_t name;
  ptr_t base_method_list;
  ptr_t base_protocols;
  ptr_t ivars;
  ptr_t weak_ivar_layout;
  ptr_t base_properties;
};


struct class_rw_ext_t {
  class_ro_t* ro;
};

struct class_rw_t {
  using ro_or_rw_ext_t = PointerUnion<const class_ro_t, class_rw_ext_t>;
  uint32_t flags;
  uint16_t witness;
  uint16_t index;
  ro_or_rw_ext_t ro_or_rw_ext;

  inline const class_ro_t* ro() const {
    if (ro_or_rw_ext.is<class_rw_ext_t>()) {
    return ro_or_rw_ext.get<const class_rw_ext_t*>()->ro;
    }
    return ro_or_rw_ext.get<const class_ro_t*>();
  }
};

template<class T>
struct method_t {
  T name;  // ptr_t -> SEL
  T types;
  T imp;
};

using small_method_t = method_t<int32_t>;
using big_method_t   = method_t<uintptr_t>;

struct cache_t {
  using mask_t = uint32_t; // uint16_t on x86

  uintptr_t buckets_and_maybe_mask;
  union {
    struct {
      mask_t   maybe_mask;
      uint16_t flags;
      uint16_t occupied;
    };
  };
  uintptr_t original_preopt_cache;
};

struct class_data_bits_t {
  static constexpr auto FAST_DATA_MASK = 0x00007ffffffffff8UL;
  static constexpr auto RW_REALIZED = 1 << 31;
  uintptr_t bits;

  inline uintptr_t class_ro_ptr() const {
    return bits & FAST_DATA_MASK;
  }

  inline uintptr_t class_ro_ptr2() const {
    const auto* maybe_rw = reinterpret_cast<const class_rw_t*>(class_ro_ptr());
    return (uintptr_t)maybe_rw->ro();
  }




  //inline class_ro_t class_ro(MachOStream& stream) const {
  //  return stream.peek<class_ro_t>(bits & FAST_DATA_MASK);
  //}
};


struct objc_class_t : objc_object_t {
  /*
   * See objc-runtime-new.h
   */
  uintptr_t         super_class;
  cache_t           cache;
  class_data_bits_t bits;
};

static_assert(sizeof(objc_class_t) == 0x28);


}
#endif
