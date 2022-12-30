include(FetchContent)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

FetchContent_Declare(
  nanobind
  GIT_REPOSITORY https://github.com/wjakob/nanobind.git
  GIT_TAG        a7d8dc08657ada946e174b849686dc969376ffc6
)
FetchContent_GetProperties(nanobind)
if(NOT nanobind_POPULATED)
  FetchContent_Populate(nanobind)
  add_subdirectory(${nanobind_SOURCE_DIR} ${nanobind_BINARY_DIR}
                   EXCLUDE_FROM_ALL)
endif()



nanobind_add_module(
  iCDump
  NB_STATIC
)

include(${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt)


target_include_directories(iCDump
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(iCDump PRIVATE LIB_ICDUMP)

set(_PY_ICDUMP_SUFFIX )

if(UNIX)
  set(_PY_ICDUMP_SUFFIX ".so")
elseif(WIN32)
  set(_PY_ICDUMP_SUFFIX ".pyd")
else()
  message(FATAL_ERROR "Platform unknown")
endif()

set_target_properties(iCDump PROPERTIES
  CXX_STANDARD              17
  CXX_VISIBILITY_PRESET     hidden
  C_VISIBILITY_PRESET       hidden
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME               "icdump"
  PREFIX                    ""
  SUFFIX ${_PY_ICDUMP_SUFFIX}
)

target_compile_options(iCDump PRIVATE
  -fdata-sections -ffunction-sections
  -fvisibility-inlines-hidden
)


