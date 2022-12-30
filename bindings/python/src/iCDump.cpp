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
#include <iCDump/iCDump.hpp>
#include <iCDump/Logging.hpp>
#include <iCDump/version.h>

#include "ObjC.hpp"

namespace nb = nanobind;

using namespace nb::literals;
using namespace iCDump;

NB_MODULE(icdump, m) {

  m.attr("__version__")   = nb::str(ICDUMP_VERSION);
  m.attr("__tag__")       = nb::str(ICDUMP_TAG);
  m.attr("__commit__")    = nb::str(ICDUMP_COMMIT);

  nb::enum_<LOG_LEVEL>(m, "LOG_LEVEL")
    .value("TRACE",    LOG_LEVEL::TRACE)
    .value("DEBUG",    LOG_LEVEL::DEBUG)
    .value("INFO",     LOG_LEVEL::INFO)
    .value("WARN",     LOG_LEVEL::WARN)
    .value("ERR",      LOG_LEVEL::ERR)
    .value("CRITICAL", LOG_LEVEL::CRITICAL);

  nb::enum_<ARCH>(m, "ARCH")
    .value("AUTO",    ARCH::AUTO)
    .value("AARCH64", ARCH::AARCH64)
    .value("ARM",     ARCH::ARM)
    .value("X86_64",  ARCH::X86_64)
    .value("X86",     ARCH::X86);

  m.def("disable_log", &disable_log);
  m.def("enable_log", &enable_log);
  m.def("set_log_level", &set_log_level);


  nb::module_ m_objc = m.def_submodule("objc", "iCDump Objective-C module");
  py::ObjC::init(m_objc);
}
