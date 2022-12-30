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
#ifndef ICDUMP_TYPESENC_H_
#define ICDUMP_TYPESENC_H_
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <set>

namespace iCDump::ObjC {

enum class OBJC_TYPES {
  UNKNOWN = 0,
  // Start of PrimitiveTy
  CHAR,
  INT,
  SHORT,
  LONG,
  LONG_LONG,
  UNSIGNED_CHAR,
  UNSIGNED_INT,
  UNSIGNED_SHORT,
  UNSIGNED_LONG,
  UNSIGNED_LONG_LONG,
  FLOAT,
  DOUBLE,
  BOOL,
  VOID,
  CSTRING,
  // End of PrimitiveTy
  OBJECT,
  CLASS,
  SELECTOR,
  ARRAY,
  STRUCT,
  UNION,
  BIT_FIELD,
  POINTER,
  BLOCK,
};

const char* to_string(OBJC_TYPES type);


enum class OBJC_TYPE_SPECIFIERS {
  UNKNOWN = 0,
  CONST,
  IN,
  IN_OUT,
  OUT,
  BY_COPY,
  BY_REF,
  ONE_WAY,
  ATOMIC,
  COMPLEX,
};


enum class OBJC_PROP_SPECIFIERS {
  UNKNOWN = 0,
  READ_ONLY,
  COPY,
  REF,
  NON_ATOMIC,
  DYNAMIC,
  WEAK,
  GARBAGE,
};

struct Type {
  OBJC_TYPES type;
  std::set<OBJC_TYPE_SPECIFIERS> specifiers;
};

struct PrimitiveTy : public Type {
  PrimitiveTy(OBJC_TYPES t) : Type{t} {}

  inline static bool classof(const Type& t) {
    return OBJC_TYPES::CHAR <= t.type && t.type <= OBJC_TYPES::CSTRING;
  }
};

struct SelectorTy : Type {
  SelectorTy() : Type{OBJC_TYPES::SELECTOR} {}
};

struct UnknownTy : Type {
  UnknownTy() : Type{OBJC_TYPES::UNKNOWN} {}
};

struct ClassTy : Type {
  ClassTy() : Type{OBJC_TYPES::CLASS} {}
};

struct BlockTy : Type {
  BlockTy() : Type{OBJC_TYPES::BLOCK} {}
};

struct ObjectTy : Type {
  ObjectTy(std::string obj_name) :
    Type{OBJC_TYPES::OBJECT}, name(std::move(obj_name)) {}
  std::string name;
};

struct PointerTy : Type {
  PointerTy(std::unique_ptr<Type> t) :
    Type{OBJC_TYPES::POINTER}, subtype(std::move(t)) {}
  std::unique_ptr<Type> subtype;
};

struct ArrayTy : Type {
  ArrayTy(size_t dim, std::unique_ptr<Type> subtype) :
    Type{OBJC_TYPES::ARRAY}, dim(dim), subtype(std::move(subtype)) {}
  size_t dim;
  std::unique_ptr<Type> subtype;
};

struct AttrTy {
  AttrTy(std::string name, std::unique_ptr<Type> t) :
    name(std::move(name)), type(std::move(t)) {}
  std::string name;
  std::unique_ptr<Type> type;
};

struct StructTy : Type {
  using attributes_t = std::vector<AttrTy>;

  StructTy() = delete;
  StructTy(const StructTy&) = delete;
  StructTy& operator=(const StructTy&) = delete;

  StructTy(std::string name, attributes_t attrs) :
    Type{OBJC_TYPES::STRUCT}, name(name), attributes(std::move(attrs)) {}

  std::string name;
  attributes_t attributes;
};

struct UnionTy : Type {
  using attributes_t = std::vector<AttrTy>;

  UnionTy() = delete;
  UnionTy(const UnionTy&) = delete;
  UnionTy& operator=(const UnionTy&) = delete;

  UnionTy(std::string name, attributes_t attrs) :
    Type{OBJC_TYPES::UNION}, name(name), attributes(std::move(attrs)) {}

  std::string name;
  attributes_t attributes;
};

struct BitFieldTy : Type {
  BitFieldTy(size_t size) :
    Type{OBJC_TYPES::BIT_FIELD}, size(size) {}
  size_t size;
};


static const std::unordered_map<char, OBJC_TYPES> OBJC_TYPES_ID {
  {'c', OBJC_TYPES::CHAR},
  {'i', OBJC_TYPES::INT},
  {'s', OBJC_TYPES::SHORT},
  {'l', OBJC_TYPES::LONG},
  {'q', OBJC_TYPES::LONG_LONG},
  {'C', OBJC_TYPES::UNSIGNED_CHAR},
  {'I', OBJC_TYPES::UNSIGNED_INT},
  {'S', OBJC_TYPES::UNSIGNED_SHORT},
  {'L', OBJC_TYPES::UNSIGNED_LONG},
  {'Q', OBJC_TYPES::UNSIGNED_LONG_LONG},
  {'f', OBJC_TYPES::FLOAT},
  {'d', OBJC_TYPES::DOUBLE},
  {'B', OBJC_TYPES::BOOL},
  {'v', OBJC_TYPES::VOID},
  {'*', OBJC_TYPES::CSTRING},
  {'@', OBJC_TYPES::OBJECT},
  {'#', OBJC_TYPES::CLASS},
  {':', OBJC_TYPES::SELECTOR},
  {'[', OBJC_TYPES::ARRAY},
  {'{', OBJC_TYPES::STRUCT},
  {'(', OBJC_TYPES::UNION},
  {'b', OBJC_TYPES::BIT_FIELD},
  {'^', OBJC_TYPES::POINTER},
  {'?', OBJC_TYPES::UNKNOWN},
};

static const std::unordered_map<char, OBJC_TYPE_SPECIFIERS> OBJC_TSPECIFIERS_ID {
  {'r', OBJC_TYPE_SPECIFIERS::CONST},
  {'n', OBJC_TYPE_SPECIFIERS::IN},
  {'N', OBJC_TYPE_SPECIFIERS::IN_OUT},
  {'o', OBJC_TYPE_SPECIFIERS::OUT},
  {'O', OBJC_TYPE_SPECIFIERS::BY_COPY},
  {'R', OBJC_TYPE_SPECIFIERS::BY_REF},
  {'V', OBJC_TYPE_SPECIFIERS::ONE_WAY},
};

static const std::unordered_map<char, OBJC_PROP_SPECIFIERS> OBJC_PROP_SPECIFIERS_ID {
  {'R', OBJC_PROP_SPECIFIERS::READ_ONLY},
  {'C', OBJC_PROP_SPECIFIERS::COPY},
  {'&', OBJC_PROP_SPECIFIERS::REF},
  {'N', OBJC_PROP_SPECIFIERS::NON_ATOMIC},
  {'D', OBJC_PROP_SPECIFIERS::DYNAMIC},
  {'W', OBJC_PROP_SPECIFIERS::WEAK},
  {'P', OBJC_PROP_SPECIFIERS::GARBAGE},
};

using types_t = std::vector<std::unique_ptr<Type>>;
using type_specifiers_t = std::vector<OBJC_TYPE_SPECIFIERS>;

types_t decode_type(const std::string& encoded);
}
#endif
