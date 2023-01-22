cmake_minimum_required(VERSION 3.15)
project(iCDump VERSION 1.0.0 LANGUAGES CXX)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

include(CMakePackageConfigHelpers)

option(ICDUMP_LLVM ON)
option(ICDUMP_PYTHON_BINDINGS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git"
   AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  find_package(Git REQUIRED)
  include(iCDumpGit)
else()
  set(ICDUMP_VERSION_MAJOR "1")
  set(ICDUMP_VERSION_MINOR "0")
  set(ICDUMP_VERSION_PATCH "0")
endif()

project(iCDump
        VERSION ${ICDUMP_VERSION_MAJOR}.${ICDUMP_VERSION_MINOR}.${ICDUMP_VERSION_PATCH}
        LANGUAGES CXX)

set(ICDUMP_LLVM_SUPPORT 0)
message(STATUS "ICDUMP_LLVM: ${ICDUMP_LLVM}")
if(ICDUMP_LLVM)
  find_package(LLVM 14 CONFIG)
  message(STATUS "LLVM Found: ${LLVM_FOUND}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  set(ICDUMP_LLVM_SUPPORT 0)
  if(LLVM_FOUND)
    set(ICDUMP_LLVM_SUPPORT 1)
    find_package(Clang REQUIRED CONFIG)
    message(STATUS "ClangConfig Dir: ${CLANG_CMAKE_DIR}")
    if(NOT "14" VERSION_EQUAL "${LLVM_VERSION_MAJOR}")
      message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 14")
    endif()

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  else()
    #set(ICDUMP_LLVM OFF)
  endif()
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/iCDump/config.hpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/iCDump/config.hpp")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/iCDump/version.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/iCDump/version.h")

# CCACHE
# ======
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)
  message(STATUS "Found ccache: ${CCACHE_FOUND}")
else()
  find_program(SCCACHE_FOUND sccache)
  if (SCCACHE_FOUND)
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    message(STATUS "Found sccache: ${SCCACHE_FOUND}")
  else()
    message(WARNING "Can't find cache solution")
  endif()
endif()


set(ICDUMP_READER_THIRD_PARTY_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third-party/")

find_package(ZLIB)
find_package(LIEF REQUIRED)
include(spdlog)

add_library(LIB_ICDUMP)
add_library(iCDump::iCDump ALIAS LIB_ICDUMP)

target_sources(LIB_ICDUMP
  PRIVATE
  src/log.cpp
  src/log_public.cpp
  src/iCDump.cpp
  src/MachOStream.cpp
)

# Objective C Engine
target_sources(LIB_ICDUMP
  PRIVATE
  src/ObjC/Class.cpp
  src/ObjC/IVar.cpp
  src/ObjC/Metadata.cpp
  src/ObjC/Method.cpp
  src/ObjC/Parser.cpp
  src/ObjC/Property.cpp
  src/ObjC/Protocol.cpp
  src/ObjC/TypesEncoding.cpp
)


# Clang AST Generator
if(ICDUMP_LLVM_SUPPORT)
  include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
  target_sources(LIB_ICDUMP
    PRIVATE
    src/ClangAST/utils.cpp
    src/ClangAST/ASTGen.cpp
  )

  target_sources(LIB_ICDUMP
    PRIVATE
    src/llvm-swift/swift/lib/Demangling/Demangler.cpp
    src/llvm-swift/swift/lib/Demangling/Context.cpp
    src/llvm-swift/swift/lib/Demangling/ManglingUtils.cpp
    src/llvm-swift/swift/lib/Demangling/NodeDumper.cpp
    src/llvm-swift/swift/lib/Demangling/NodePrinter.cpp
    src/llvm-swift/swift/lib/Demangling/OldDemangler.cpp
    src/llvm-swift/swift/lib/Demangling/OldRemangler.cpp
    src/llvm-swift/swift/lib/Demangling/Punycode.cpp
    src/llvm-swift/swift/lib/Demangling/Remangler.cpp
  )
endif()

add_library(LIEF INTERFACE)
add_library(iCDump::LIEF ALIAS LIEF)
target_link_libraries(LIEF INTERFACE $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib/libLIEF${CMAKE_STATIC_LIBRARY_SUFFIX}>)

target_link_libraries(LIB_ICDUMP PRIVATE $<BUILD_INTERFACE:LIEF::LIEF>)

target_include_directories(LIB_ICDUMP
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    src/llvm-swift/swift/include/
)

if(ICDUMP_LLVM_SUPPORT)
  target_include_directories(LIB_ICDUMP
    PRIVATE
      src/llvm-swift/swift/include/
  )
endif()

target_compile_definitions(LIB_ICDUMP
  PRIVATE
    SPDLOG_NO_EXCEPTIONS # Required for iOS
    SPDLOG_NO_THREAD_ID  # Required for iOS
    LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1
    "LLVM_ATTRIBUTE_NORETURN=__attribute__((noreturn))"
    SWIFT_SUPPORT_OLD_MANGLING=1
)

set_target_properties(LIB_ICDUMP PROPERTIES
  CXX_STANDARD              17
  CXX_STANDARD_REQUIRED     OFF
  CXX_EXTENSIONS            OFF
  POSITION_INDEPENDENT_CODE ON
  OUTPUT_NAME               iCDump
  EXPORT_NAME               iCDump
  CLEAN_DIRECT_OUTPUT       1
)


target_compile_options(LIB_ICDUMP PRIVATE
  -fdata-sections -ffunction-sections
  -fno-rtti -fno-exceptions
  -fvisibility-inlines-hidden
)

target_link_libraries(LIB_ICDUMP PRIVATE
  spdlog
)

if(ICDUMP_LLVM_SUPPORT)
  llvm_map_components_to_libnames(llvm_libs support core)
  target_link_libraries(LIB_ICDUMP PRIVATE
    ${llvm_libs}
    clangBasic
    clangCodeGen
  )
endif()

if(ICDUMP_PYTHON_BINDINGS)
  add_subdirectory(bindings/python)
endif()

# Find Package Config
# ======================
configure_file(
  iCDumpConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/iCDumpConfig.cmake
  @ONLY)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/iCDumpConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)


if(UNIX)
  include(GNUInstallDirs)
  set(CMAKE_INSTALL_LIBDIR "lib")
else()
  if(WIN32)
      set(CMAKE_INSTALL_LIBDIR      "lib")
      set(CMAKE_INSTALL_DATADIR     "share")
      set(CMAKE_INSTALL_INCLUDEDIR  "include")
      set(CMAKE_INSTALL_BINDIR      "bin")
      set(CMAKE_INSTALL_DATAROOTDIR "share")
      message(STATUS "Setting installation destination on Windows to: ${CMAKE_INSTALL_PREFIX}")
    else()
      message(FATAL_ERROR "System not UNIX nor WIN32 - not implemented yet")
    endif()
endif()

get_target_property(_LIEF_INC  LIEF::LIEF INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(_LIEF_CONF LIEF::LIEF IMPORTED_CONFIGURATIONS)
get_target_property(_LIEF_LIB  LIEF::LIEF IMPORTED_LOCATION_${_LIEF_CONF})

message(STATUS "LIEF Lib      : ${_LIEF_LIB}")
message(STATUS "LIEF Includes : ${_LIEF_INC}")

install(
  TARGETS LIB_ICDUMP spdlog LIEF
  EXPORT iCDumpExport
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  FILES       ${_LIEF_LIB}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT   libraries)

install(
  DIRECTORY   ${_LIEF_INC}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT   headers)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/iCDumpConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/iCDumpConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/iCDump/cmake
  COMPONENT config)

if(BUILD_SHARED_LIBS)
  set(lib_type shared)
else()
  set(lib_type static)
endif()

install(
  EXPORT      iCDumpExport
  NAMESPACE   iCDump::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/iCDump/cmake
  FILE        iCDumpExport-${lib_type}.cmake
  COMPONENT   config)

install(
  DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_BINARY_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers)

export(
  EXPORT iCDumpExport
  NAMESPACE iCDump::
  FILE iCDumpExport-${lib_type}.cmake)


