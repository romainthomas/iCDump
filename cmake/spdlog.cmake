if(__add_spdlog)
  return()
endif()
set(__add_spdlog ON)
include(ExternalProject)

# spdlog
# ======
set(SPDLOG_VERSION 1.11.0)
set(SPDLOG_URL "${ICDUMP_READER_THIRD_PARTY_DIRECTORY}/spdlog-${SPDLOG_VERSION}.zip")
set(SPDLOG_SHA256 SHA256=33f83c6b86ec0fbbd0eb0f4e980da6767494dc0ad063900bcfae8bc3e9c75f21)
ExternalProject_Add(spdlog-project
  URL               ${SPDLOG_URL}
  URL_HASH          ${SPDLOG_SHA256}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   "")
ExternalProject_get_property(spdlog-project SOURCE_DIR)
set(SPDLOG_SOURCE_DIR "${SOURCE_DIR}")

add_library(spdlog INTERFACE)
add_dependencies(spdlog spdlog-project)
target_include_directories(spdlog SYSTEM INTERFACE "$<BUILD_INTERFACE:${SPDLOG_SOURCE_DIR}/include/>")
