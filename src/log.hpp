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
#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/chrono.h>

#define ICDUMP_TRACE(...) iCDump::Logger::trace(__VA_ARGS__)
#define ICDUMP_DEBUG(...) iCDump::Logger::debug(__VA_ARGS__)
#define ICDUMP_INFO(...)  iCDump::Logger::info(__VA_ARGS__)
#define ICDUMP_WARN(...)  iCDump::Logger::warn(__VA_ARGS__)
#define ICDUMP_ERR(...)   iCDump::Logger::err(__VA_ARGS__)

namespace iCDump {
class Logger {
  public:
  enum class LEVEL {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERR,
    CRITICAL,
  };
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  static Logger& instance();

  static void set_level(LEVEL lvl);

  static void disable();

  static void enable();

  template <typename... Args>
  static void trace(const char *fmt, const Args &... args) {
    Logger::instance().sink_->trace(fmt, args...);
  }

  template <typename... Args>
  static void debug(const char *fmt, const Args &... args) {
    Logger::instance().sink_->debug(fmt, args...);
  }

  template <typename... Args>
  static void info(const char *fmt, const Args &... args) {
    Logger::instance().sink_->info(fmt, args...);
  }

  template <typename... Args>
  static void err(const char *fmt, const Args &... args) {
    Logger::instance().sink_->error(fmt, args...);
  }

  template <typename... Args>
  static void warn(const char *fmt, const Args &... args) {
    Logger::instance().sink_->warn(fmt, args...);
  }

  ~Logger();
  private:
  Logger(void);
  Logger(Logger&&);
  Logger& operator=(Logger&&);

  static void destroy(void);
  inline static Logger* instance_ = nullptr;
  std::shared_ptr<spdlog::logger> sink_;
};
}
