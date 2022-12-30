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
#include "iCDump/ObjC/TypesEncoding.hpp"
#include "log.hpp"

namespace iCDump::ObjC {

inline bool is_digit(char c) {
  return '0' <= c && c <= '9';
}

inline size_t get_pos(const std::string& encoded, std::string::const_iterator& it) {
  return std::distance(std::begin(encoded), it);
}

OBJC_TYPE_SPECIFIERS process_specifier(const std::string& encoded, std::string::const_iterator& it);
std::unique_ptr<Type> process(const std::string& encoded, std::string::const_iterator& it);

//std::string object_name(const std::string& encoded, std::string::const_iterator& it, size_t& pos) {
//  char quote = *it;
//  ++it;
//  ICDUMP_INFO("{}", encoded.substr(get_pos(encoded, it)));
//  size_t token_pos = encoded.find_first_of('"', pos);
//  if (token_pos == std::string::npos) {
//    ICDUMP_ERR("Can't find '\"' token");
//    return "";
//  }
//  std::string name = encoded.substr(pos, token_pos - pos);
//  const size_t delta = name.size() + 1; // +1 for the ending '"'
//  pos += delta;
//  it  += delta;
//  return name;
//}

void skip_digits(const std::string& encoded, std::string::const_iterator& it) {
  while (it != std::end(encoded) && is_digit(*it)) {
    ++it;
  }
}

std::string read_opt_name(const std::string& encoded, std::string::const_iterator& it) {
  static constexpr char DELIM = '"';
  if (it == std::end(encoded) || *it != DELIM) {
    return {};
  }
  char __delim = *it++;
  std::string name;
  for (; it != std::end(encoded) && *it != DELIM; ++it) {
    name += *it;
  }
  if (it != std::end(encoded) && *it == DELIM) {
    ++it;
  }
  return name;
}

std::unique_ptr<StructTy> process_structure(const std::string& encoded, std::string::const_iterator& it) {
  static constexpr char NAME_DELIM       = '=';
  static constexpr char STRUCT_END_DELIM = '}';
  //ICDUMP_DEBUG("Reading structure from {}", encoded.substr(pos));
  std::string struct_name;
  if (it != std::end(encoded) && *it == '?') {
    // struct_name = "anonymous";
    ++it;
  }
  while (it != std::end(encoded) && *it != NAME_DELIM && *it != STRUCT_END_DELIM) {
    struct_name += *it++;
  }

  //ICDUMP_INFO("Struct name: {}", struct_name);

  if (it != std::end(encoded) && *it == STRUCT_END_DELIM) {
    // No field eg. ^{MyStruct}
    ++it;
    return std::make_unique<StructTy>(struct_name, StructTy::attributes_t{});
  }

  if (it != std::end(encoded) && *it == NAME_DELIM) {
    ++it;
  }
  StructTy::attributes_t attributes;
  if (it != std::end(encoded) && *it == STRUCT_END_DELIM) {
    ++it;
  } else {
    //ICDUMP_DEBUG("Reading fields from {}", encoded.substr(pos));
    while(it != std::end(encoded) && *it != STRUCT_END_DELIM) {
      std::string name = read_opt_name(encoded, it);
      std::unique_ptr<Type> field_type = process(encoded, it);
      attributes.emplace_back(name, std::move(field_type));
    }
    if (it != std::end(encoded) && *it == STRUCT_END_DELIM) {
      ++it;
      //ICDUMP_DEBUG("End struct");
      return std::make_unique<StructTy>(struct_name, std::move(attributes));
    }
    ICDUMP_ERR("Expecting token '}}' ('{}')", *it);
    return nullptr;
  }
  return std::make_unique<StructTy>(struct_name, std::move(attributes));
}

std::unique_ptr<UnionTy> process_union(const std::string& encoded, std::string::const_iterator& it) {
  static constexpr char NAME_DELIM       = '=';
  static constexpr char UNION_END_DELIM = ')';
  //ICDUMP_DEBUG("Reading union from {}", encoded.substr(pos));
  std::string union_name;
  if (it != std::end(encoded) && *it == '?') {
    // Anonymous union
    //union_name = "anonymous";
    ++it;
  }
  while (it != std::end(encoded) && *it != NAME_DELIM) {
    union_name += *it++;
  }
  if (it != std::end(encoded) && *it == NAME_DELIM) {
    ++it;
  }
  //ICDUMP_INFO("Union name: {}", union_name);

  UnionTy::attributes_t attributes;
  if (it != std::end(encoded) && *it == UNION_END_DELIM) {
    ++it;
  } else {
    //ICDUMP_DEBUG("Reading fields from {}", encoded.substr(pos));
    while(it != std::end(encoded) && *it != UNION_END_DELIM) {
      std::string name = read_opt_name(encoded, it);
      //if (!name.empty()) {
      //  ICDUMP_DEBUG("Field: {}", name);
      //}
      std::unique_ptr<Type> field_type = process(encoded, it);
      attributes.emplace_back(name, std::move(field_type));
    }
    if (it != std::end(encoded) && *it == UNION_END_DELIM) {
      ++it;
      //ICDUMP_DEBUG("End union");
      return std::make_unique<UnionTy>(union_name, std::move(attributes));
    }
    // error
    return nullptr;

  }
  return std::make_unique<UnionTy>(union_name, std::move(attributes));
}

std::string read_str_number(const std::string& encoded, std::string::const_iterator& it) {
  std::string n;
  while (it != std::end(encoded) && is_digit(*it)) {
    n = *it++;
  }
  return n;
}

std::unique_ptr<BitFieldTy> process_bitfield(const std::string& encoded,
                                             std::string::const_iterator& it) {
  const std::string num_str = read_str_number(encoded, it);
  if (num_str.empty()) {
    ICDUMP_ERR("num_str is null");
    return nullptr;
  }
  char* endp;
  size_t fsize = std::strtoull(num_str.c_str(), &endp, 10);
  return std::make_unique<BitFieldTy>(fsize);
}



std::unique_ptr<ArrayTy> process_array(const std::string& encoded, std::string::const_iterator& it) {
  const std::string array_size_str = read_str_number(encoded, it);
  if (array_size_str.empty()) {
    ICDUMP_ERR("Arraysize is null");
    return nullptr;
  }
  char* endp;
  size_t array_size = std::strtoull(array_size_str.c_str(), &endp, 10);

  //ICDUMP_DEBUG("Reading array type from {}", encoded.substr(pos));
  std::unique_ptr<Type> underlying_type;
  while(it != std::end(encoded) && *it != ']') {
    underlying_type = process(encoded, it);
  }

  if (it != std::end(encoded) && *it == ']') {
    ++it;
    return std::make_unique<ArrayTy>(array_size, std::move(underlying_type));
  }
  ICDUMP_ERR("Can't find closing array token");
  return nullptr;
}

std::unique_ptr<Type> process(const std::string& encoded, std::string::const_iterator& it) {
  skip_digits(encoded, it);

  //ICDUMP_DEBUG("Reading token at {:d}: {}", pos, encoded.substr(pos));

  if (it == std::end(encoded)) {
    return nullptr;
  }

  {
    OBJC_TYPE_SPECIFIERS spec = OBJC_TYPE_SPECIFIERS::UNKNOWN;
    do {
      spec = process_specifier(encoded, it);
    } while (it != std::end(encoded) && spec != OBJC_TYPE_SPECIFIERS::UNKNOWN);

    if (it == std::end(encoded)) {
      return nullptr;
    }
  }
  const char c = *it++;
  switch (c) {
    case 'c': return std::make_unique<PrimitiveTy>(OBJC_TYPES::CHAR);
    case 'i': return std::make_unique<PrimitiveTy>(OBJC_TYPES::INT);
    case 's': return std::make_unique<PrimitiveTy>(OBJC_TYPES::SHORT);
    case 'l': return std::make_unique<PrimitiveTy>(OBJC_TYPES::LONG);
    case 'q': return std::make_unique<PrimitiveTy>(OBJC_TYPES::LONG_LONG);
    case 'C': return std::make_unique<PrimitiveTy>(OBJC_TYPES::UNSIGNED_CHAR);
    case 'I': return std::make_unique<PrimitiveTy>(OBJC_TYPES::UNSIGNED_INT);
    case 'S': return std::make_unique<PrimitiveTy>(OBJC_TYPES::UNSIGNED_SHORT);
    case 'L': return std::make_unique<PrimitiveTy>(OBJC_TYPES::UNSIGNED_LONG);
    case 'Q': return std::make_unique<PrimitiveTy>(OBJC_TYPES::UNSIGNED_LONG_LONG);
    case 'f': return std::make_unique<PrimitiveTy>(OBJC_TYPES::FLOAT);
    case 'd': return std::make_unique<PrimitiveTy>(OBJC_TYPES::DOUBLE);
    case 'B': return std::make_unique<PrimitiveTy>(OBJC_TYPES::BOOL);
    case 'v': return std::make_unique<PrimitiveTy>(OBJC_TYPES::VOID);
    case '*': return std::make_unique<PrimitiveTy>(OBJC_TYPES::CSTRING);
    case '#': return std::make_unique<ClassTy>();
    case ':': return std::make_unique<SelectorTy>();
    case '[': return process_array(encoded, it);
    case '{': return process_structure(encoded, it);
    case '(': return process_union(encoded, it);
    case 'b': return process_bitfield(encoded, it);
    case '?': return std::make_unique<UnknownTy>();
    case '^':
      {
        // Special case: ^? --> void*
        if (it != std::end(encoded) && *it == '?') {
          ++it;
          return std::make_unique<PointerTy>(std::make_unique<PrimitiveTy>(OBJC_TYPES::VOID));
        }
        const size_t current_pos = get_pos(encoded, it);
        std::unique_ptr<Type> ptr_type = process(encoded, it);
        if (ptr_type == nullptr) {
          ICDUMP_ERR("Can't resolve pointer type: {}", encoded.substr(current_pos, 3));
          return nullptr;
        }
        return std::make_unique<PointerTy>(std::move(ptr_type));

      }
    case '@':
      {
        // Special case: @? --> void*
        if (it != std::end(encoded) && *it == '?') {
          ++it;
          return std::make_unique<BlockTy>();
        }
        std::string name = read_opt_name(encoded, it);
        return std::make_unique<ObjectTy>(name);
      }
    default:
      {
        size_t pos = get_pos(encoded, it);
        ICDUMP_ERR("Unsupported type: {} ({})", c, encoded.substr(pos - 1));
        return nullptr;
      }
  }
  return nullptr;
}

OBJC_TYPE_SPECIFIERS process_specifier(const std::string& encoded, std::string::const_iterator& it) {
  if (it == std::end(encoded)) {
    return OBJC_TYPE_SPECIFIERS::UNKNOWN;
  }
  const char c = *it;
  switch (c) {
    case 'r': ++it; return OBJC_TYPE_SPECIFIERS::CONST;
    case 'n': ++it; return OBJC_TYPE_SPECIFIERS::IN;
    case 'N': ++it; return OBJC_TYPE_SPECIFIERS::IN_OUT;
    case 'o': ++it; return OBJC_TYPE_SPECIFIERS::OUT;
    case 'O': ++it; return OBJC_TYPE_SPECIFIERS::BY_COPY;
    case 'R': ++it; return OBJC_TYPE_SPECIFIERS::BY_REF;
    case 'V': ++it; return OBJC_TYPE_SPECIFIERS::ONE_WAY;
    case 'A': ++it; return OBJC_TYPE_SPECIFIERS::ATOMIC;
    case 'j': ++it; return OBJC_TYPE_SPECIFIERS::COMPLEX;
    default:        return OBJC_TYPE_SPECIFIERS::UNKNOWN;
  }

}

types_t decode_type(const std::string& encoded) {
  if (encoded.empty()) {
    return {};
  }

  size_t i = 0;
  std::string::const_iterator it = std::begin(encoded);
  const size_t encoded_size = encoded.size();
  std::vector<std::unique_ptr<Type>> types;
  while (it != std::end(encoded)) {
    OBJC_TYPE_SPECIFIERS specifier = process_specifier(encoded, it);
    std::unique_ptr<Type> decoded = process(encoded, it);
    if (decoded) {
      types.push_back(std::move(decoded));
    }
  }
  return types;
}

const char* to_string(OBJC_TYPES type) {
  switch (type) {
    case OBJC_TYPES::CHAR:                return "CHAR";
    case OBJC_TYPES::INT:                 return "INT";
    case OBJC_TYPES::SHORT:               return "SHORT";
    case OBJC_TYPES::LONG:                return "LONG";
    case OBJC_TYPES::LONG_LONG:           return "LONG_LONG";
    case OBJC_TYPES::UNSIGNED_CHAR:       return "UNSIGNED_CHAR";
    case OBJC_TYPES::UNSIGNED_INT:        return "UNSIGNED_INT";
    case OBJC_TYPES::UNSIGNED_SHORT:      return "UNSIGNED_SHORT";
    case OBJC_TYPES::UNSIGNED_LONG:       return "UNSIGNED_LONG";
    case OBJC_TYPES::UNSIGNED_LONG_LONG:  return "UNSIGNED_LONG_LONG";
    case OBJC_TYPES::FLOAT:               return "FLOAT";
    case OBJC_TYPES::DOUBLE:              return "DOUBLE";
    case OBJC_TYPES::BOOL:                return "BOOL";
    case OBJC_TYPES::VOID:                return "VOID";
    case OBJC_TYPES::CSTRING:             return "CSTRING";
    case OBJC_TYPES::OBJECT:              return "OBJECT";
    case OBJC_TYPES::CLASS:               return "CLASS";
    case OBJC_TYPES::SELECTOR:            return "SELECTOR";
    case OBJC_TYPES::ARRAY:               return "ARRAY";
    case OBJC_TYPES::STRUCT:              return "STRUCT";
    case OBJC_TYPES::UNION:               return "UNION";
    case OBJC_TYPES::BIT_FIELD:           return "BIT_FIELD";
    case OBJC_TYPES::POINTER:             return "POINTER";
    case OBJC_TYPES::BLOCK:               return "BLOCK";
    case OBJC_TYPES::UNKNOWN:             return "UNKNOWN";
  }
  return "";
}
}
