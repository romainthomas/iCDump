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
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/make_iterator.h>

#include "iCDump/iCDump.hpp"
#include "iCDump/NonCopyable.hpp"

#include "ObjC.hpp"
#include "iterator.hpp"


namespace nb = nanobind;
using namespace nb::literals;

NAMESPACE_BEGIN(NB_NAMESPACE)
NAMESPACE_BEGIN(detail)
template <> struct type_caster<iCDump::ObjC::Type> {
  NB_TYPE_CASTER(iCDump::ObjC::Type, const_name("Type"));

  bool from_python(handle src, uint8_t flags, cleanup_list* cl) noexcept {
    // TODO(romain)
    return false;
  }

  static handle from_cpp(iCDump::ObjC::Type& Ty, rv_policy rvp,
                         cleanup_list* cl) noexcept {
    using namespace iCDump::ObjC;

    if (iCDump::ObjC::PrimitiveTy::classof(Ty)) {
      return nb::detail::make_caster<iCDump::ObjC::PrimitiveTy>::from_cpp(&Ty, rvp, cl);
    }

    switch (Ty.type) {
      case OBJC_TYPES::SELECTOR:
        return nb::detail::make_caster<iCDump::ObjC::SelectorTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::UNKNOWN:
        return nb::detail::make_caster<iCDump::ObjC::UnknownTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::CLASS:
        return nb::detail::make_caster<iCDump::ObjC::ClassTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::BLOCK:
        return nb::detail::make_caster<iCDump::ObjC::BlockTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::OBJECT:
        return nb::detail::make_caster<iCDump::ObjC::ObjectTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::POINTER:
        return nb::detail::make_caster<iCDump::ObjC::PointerTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::ARRAY:
        return nb::detail::make_caster<iCDump::ObjC::ArrayTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::STRUCT:
        return nb::detail::make_caster<iCDump::ObjC::StructTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::UNION:
        return nb::detail::make_caster<iCDump::ObjC::UnionTy>::from_cpp(&Ty, rvp, cl);
      case OBJC_TYPES::BIT_FIELD:
        return nb::detail::make_caster<iCDump::ObjC::BitFieldTy>::from_cpp(&Ty, rvp, cl);
      default:
        return nb::none().release();
    }

    return nb::none().release();
  }
};
NAMESPACE_END(detail)
NAMESPACE_END(NB_NAMESPACE)

using namespace iCDump::ObjC;

namespace iCDump::py::ObjC {

struct py_types_t {
  using it_t = ref_iterator<types_t&, Type*>;

  py_types_t() = delete;
  py_types_t(const py_types_t&) = delete;
  py_types_t& operator=(const py_types_t&) = delete;

  py_types_t(py_types_t&&) = default;
  py_types_t& operator=(py_types_t&&) = default;

  py_types_t(types_t t) :
    types(std::move(t)) {}

  inline it_t items() {
    return types;
  }

  types_t types;
};

void init_types_encoding(nb::module_& m) {

  nb::enum_<OBJC_TYPES>(m, "OBJC_TYPES")
    .value("UNKNOWN", OBJC_TYPES::UNKNOWN)
    .value("CHAR", OBJC_TYPES::CHAR)
    .value("INT", OBJC_TYPES::INT)
    .value("SHORT", OBJC_TYPES::SHORT)
    .value("LONG", OBJC_TYPES::LONG)
    .value("LONG_LONG", OBJC_TYPES::LONG_LONG)
    .value("UNSIGNED_CHAR", OBJC_TYPES::UNSIGNED_CHAR)
    .value("UNSIGNED_INT", OBJC_TYPES::UNSIGNED_INT)
    .value("UNSIGNED_SHORT", OBJC_TYPES::UNSIGNED_SHORT)
    .value("UNSIGNED_LONG", OBJC_TYPES::UNSIGNED_LONG)
    .value("UNSIGNED_LONG_LONG", OBJC_TYPES::UNSIGNED_LONG_LONG)
    .value("FLOAT", OBJC_TYPES::FLOAT)
    .value("DOUBLE", OBJC_TYPES::DOUBLE)
    .value("BOOL", OBJC_TYPES::BOOL)
    .value("VOID", OBJC_TYPES::VOID)
    .value("CSTRING", OBJC_TYPES::CSTRING)
    .value("OBJECT", OBJC_TYPES::OBJECT)
    .value("CLASS", OBJC_TYPES::CLASS)
    .value("SELECTOR", OBJC_TYPES::SELECTOR)
    .value("ARRAY", OBJC_TYPES::ARRAY)
    .value("STRUCT", OBJC_TYPES::STRUCT)
    .value("UNION", OBJC_TYPES::UNION)
    .value("BIT_FIELD", OBJC_TYPES::BIT_FIELD)
    .value("POINTER", OBJC_TYPES::POINTER)
    .value("BLOCK", OBJC_TYPES::BLOCK);

  nb::enum_<OBJC_TYPE_SPECIFIERS>(m, "OBJC_TYPE_SPECIFIERS")
    .value("UNKNOWN", OBJC_TYPE_SPECIFIERS::UNKNOWN)
    .value("CONST", OBJC_TYPE_SPECIFIERS::CONST)
    .value("IN", OBJC_TYPE_SPECIFIERS::IN)
    .value("IN_OUT", OBJC_TYPE_SPECIFIERS::IN_OUT)
    .value("OUT", OBJC_TYPE_SPECIFIERS::OUT)
    .value("BY_COPY", OBJC_TYPE_SPECIFIERS::BY_COPY)
    .value("BY_REF", OBJC_TYPE_SPECIFIERS::BY_REF)
    .value("ONE_WAY", OBJC_TYPE_SPECIFIERS::ONE_WAY)
    .value("ATOMIC", OBJC_TYPE_SPECIFIERS::ATOMIC)
    .value("COMPLEX", OBJC_TYPE_SPECIFIERS::COMPLEX);

  nb::enum_<OBJC_PROP_SPECIFIERS>(m, "OBJC_PROP_SPECIFIERS")
    .value("UNKNOWN", OBJC_PROP_SPECIFIERS::UNKNOWN)
    .value("READ_ONLY", OBJC_PROP_SPECIFIERS::READ_ONLY)
    .value("COPY", OBJC_PROP_SPECIFIERS::COPY)
    .value("REF", OBJC_PROP_SPECIFIERS::REF)
    .value("NON_ATOMIC", OBJC_PROP_SPECIFIERS::NON_ATOMIC)
    .value("DYNAMIC", OBJC_PROP_SPECIFIERS::DYNAMIC)
    .value("WEAK", OBJC_PROP_SPECIFIERS::WEAK)
    .value("GARBAGE", OBJC_PROP_SPECIFIERS::GARBAGE);

  nb::class_<Type>(m, "Type")
    .def_readonly("type", &Type::type);
    //.def_readonly("specifiers", &Type::specifiers);

  nb::class_<PrimitiveTy, Type>(m, "PrimitiveTy");
  nb::class_<UnknownTy, Type>(m, "UnknownTy");
  nb::class_<SelectorTy, Type>(m, "SelectorTy");
  nb::class_<ClassTy, Type>(m, "ClassTy");
  nb::class_<BlockTy, Type>(m, "BlockTy");
  nb::class_<ObjectTy, Type>(m, "ObjectTy")
    .def_readonly("name", &ObjectTy::name);

  nb::class_<PointerTy, Type>(m, "PointerTy")
    .def_property_readonly("subtype",
        [] (PointerTy& self) {
          return self.subtype.get();
        }, nb::rv_policy::reference_internal);

  nb::class_<ArrayTy, Type>(m, "ArrayTy")
    .def_readonly("dim", &ArrayTy::dim)
    .def_property_readonly("subtype",
        [] (ArrayTy& self) {
          return self.subtype.get();
        }, nb::rv_policy::reference_internal);

  nb::class_<AttrTy>(m, "AttrTy")
    .def_readonly("name", &AttrTy::name)
    .def_property_readonly("type",
        [] (AttrTy& self) {
          return self.type.get();
        }, nb::rv_policy::reference_internal);

  using it_attributes = ref_iterator<std::vector<AttrTy>&>;
  init_iterator<it_attributes>(m, "it_attributes");

  nb::class_<StructTy, Type>(m, "StructTy")
    .def_readonly("name", &StructTy::name)
    .def_property_readonly("attributes",
        [] (StructTy& self) {
          return it_attributes{self.attributes};
        }, nb::rv_policy::move);

  nb::class_<UnionTy, Type>(m, "UnionTy")
    .def_readonly("name", &UnionTy::name)
    .def_property_readonly("attributes",
        [] (UnionTy& self) {
          return it_attributes{self.attributes};
        }, nb::rv_policy::move);

  nb::class_<BitFieldTy, Type>(m, "BitFieldTy")
    .def_readonly("size", &BitFieldTy::size);


  init_iterator<py_types_t::it_t>(m, "types_it_t");
  nb::class_<py_types_t>(m, "py_types_t")
    .def("items", &py_types_t::items, nb::keep_alive<0, 1>())
    .def("__getitem__",
      [] (py_types_t& self, size_t i) {
        if (i >= self.types.size()) {
          throw nb::index_error();
        }
        return nb::cast(self.types[i].get(), nb::rv_policy::reference);
      }, nb::rv_policy::reference)

    .def("__len__",
        [] (py_types_t& self) {
          return self.types.size();
        });

  m.def("to_string", nb::overload_cast<OBJC_TYPES>(&to_string));

  m.def("decode_type",
      [] (std::string encoded) {
        return py_types_t(decode_type(encoded));
      }, nb::rv_policy::move);
}
}
