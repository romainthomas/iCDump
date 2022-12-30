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
#include "log.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace iCDump {

enum class PLATFORMS {
  UNKNOWN = 0,
  LINUX,
  ANDROID_PLAT,
  WINDOWS,
  IOS,
  OSX,
};

constexpr PLATFORMS current_platform() {
#if defined(__ANDROID__)
  return PLATFORMS::ANDROID_PLAT;
#elif defined(__linux__)
  return PLATFORMS::LINUX;
#elif defined(_WIN64) || defined(_WIN32)
  return PLATFORMS::WINDOWS;
#elif defined(__APPLE__)
  #if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    return PLATFORMS::IOS;
  #else
    return PLATFORMS::OSX;
  #endif
#else
  return PLATFORMS::UNKNOWN;
#endif
}

Logger::Logger(Logger&&) = default;
Logger& Logger::operator=(Logger&&) = default;
Logger::~Logger() = default;

Logger::Logger(void) {
  if constexpr (current_platform() == PLATFORMS::ANDROID_PLAT) {
    #if defined(__ANDROID__)
    sink_ = spdlog::android_logger_mt("iCDump", "iCDump");
    #else
    // Should not append ...
    #endif
  }
  else if constexpr (current_platform() == PLATFORMS::IOS) {
    sink_ = spdlog::basic_logger_mt("iCDump", "/tmp/iCDump.log", /* truncate */ true);
  }
  else {
    sink_ = spdlog::stderr_color_mt("iCDump");
  }

  sink_->set_level(spdlog::level::warn);
  sink_->set_pattern("%v");
  sink_->flush_on(spdlog::level::warn);
}

Logger& Logger::instance() {
  if (instance_ == nullptr) {
    instance_ = new Logger{};
    std::atexit(destroy);
  }
  return *instance_;
}


void Logger::set_level(Logger::LEVEL lvl) {
  #define SET_LVL(L) sink->set_level(L); sink->flush_on(L);
  auto& sink = Logger::instance().sink_;
  switch (lvl) {
    case LEVEL::TRACE:    SET_LVL(spdlog::level::trace);     break;
    case LEVEL::DEBUG:    SET_LVL(spdlog::level::debug);     break;
    case LEVEL::INFO:     SET_LVL(spdlog::level::info);      break;
    case LEVEL::WARN:     SET_LVL(spdlog::level::warn);      break;
    case LEVEL::ERR:      SET_LVL(spdlog::level::err);       break;
    case LEVEL::CRITICAL: SET_LVL(spdlog::level::critical);  break;
  }
  #undef SET_LVL
}

void Logger::destroy(void) {
  delete instance_;
}

void Logger::disable(void) {
  Logger::instance().sink_->set_level(spdlog::level::off);
}

void Logger::enable(void) {
  Logger::instance().sink_->set_level(spdlog::level::warn);
}


}
