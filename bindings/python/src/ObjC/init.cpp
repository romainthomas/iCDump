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
#include <nanobind/stl/unique_ptr.h>

#include "iCDump/iCDump.hpp"

#include "ObjC.hpp"
#include "iterator.hpp"


namespace nb = nanobind;
using namespace nb::literals;

using namespace iCDump::ObjC;

namespace iCDump::py::ObjC {

void init(nb::module_& m) {
  init_types_encoding(m);

  m.def("parse", iCDump::ObjC::parse,
        "file_path"_a, "arch"_a = ARCH::AUTO);

  nb::class_<IVar>(m, "IVar")
    .def_property_readonly("name", &IVar::name)
    .def_property_readonly("mangled_type", &IVar::mangled_type)
    .def_property_readonly("type", &IVar::type, nb::rv_policy::take_ownership)
    .def("to_decl",
         &IVar::to_decl)

    .def("__str__",
         [] (const IVar& self) {
          return self.to_string();
         });

  nb::class_<Property>(m, "Property")
    .def_property_readonly("name", &Property::name)
    .def_property_readonly("attribute", &Property::attribute)
    .def("to_decl",
         &Property::to_decl)

    .def("__str__",
         [] (const Property& self) {
          return self.to_string();
         });

  nb::class_<Method> meth(m, "Method");
  nb::class_<Method::prototype_t>(meth, "prototype_t");
  meth
    .def_property_readonly("name", &Method::name)
    .def_property_readonly("mangled_type", &Method::mangled_type)
    .def_property_readonly("address", &Method::address)
    .def_property_readonly("is_instance", &Method::is_instance)
    .def_property_readonly("prototype", &Method::prototype, nb::rv_policy::move)
    .def("__str__",
         [] (const Method& self) {
          return self.to_string();
         });


  nb::class_<Protocol> protocol(m, "Protocol");
  init_iterator<Protocol::methods_it_t>(protocol, "methods_it_t");
  init_iterator<Protocol::properties_it_t>(protocol, "properties_it_t");
  protocol
    .def_property_readonly("mangled_name", &Protocol::mangled_name)
    .def_property_readonly("optional_methods", &Protocol::optional_methods, nb::rv_policy::move)
    .def_property_readonly("required_methods", &Protocol::required_methods, nb::rv_policy::move)
    .def_property_readonly("properties", &Protocol::properties, nb::rv_policy::move)
    .def("to_decl",
         &Protocol::to_decl)

    .def("__str__",
         [] (const Protocol& self) {
          return self.to_string();
         });

  nb::class_<Class> cls(m, "Class");
  /*
   * methods_it_t and properties_it_t are already registered with the Protocol class
   */
  init_iterator<Class::protocols_it_t>(cls, "protocols_it_t");
  init_iterator<Class::ivars_it_t>(cls, "ivars_it_t");

  cls
    .def_property_readonly("name", &Class::name)
    .def_property_readonly("super_class", &Class::super_class)
    .def_property_readonly("demangled_name", &Class::demangled_name)
    .def_property_readonly("is_meta", &Class::is_meta)
    .def_property_readonly("methods", &Class::methods, nb::rv_policy::move)
    .def_property_readonly("protocols", &Class::protocols, nb::rv_policy::move)
    .def_property_readonly("properties", &Class::properties, nb::rv_policy::move)
    .def_property_readonly("ivars", &Class::ivars, nb::rv_policy::move)
    .def("to_decl",
         &Class::to_decl)

    .def("__str__",
         [] (const Class& self) {
          return self.to_string();
         });

  nb::class_<Metadata> metadata(m, "Metadata");
  init_iterator<Metadata::classes_it_t>(metadata, "classes_it_t");
  init_iterator<Metadata::protocol_it_t>(metadata, "protocol_it_t");

  metadata
    .def_property_readonly("classes",
        &Metadata::classes, nb::rv_policy::move)
    .def_property_readonly("protocols",
        &Metadata::protocols, nb::rv_policy::move)
    .def("to_decl", &Metadata::to_decl);
}

}
