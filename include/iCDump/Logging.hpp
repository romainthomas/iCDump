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
#ifndef ICDUMP_LOGGING_PUBLIC_H_
#define ICDUMP_LOGGING_PUBLIC_H_
namespace iCDump {

enum class LOG_LEVEL {
  TRACE,
  DEBUG,
  INFO,
  WARN,
  ERR,
  CRITICAL,
};

void disable_log();
void enable_log();

void set_log_level(LOG_LEVEL lvl);

}
#endif
