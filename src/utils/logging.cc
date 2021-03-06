/*
  This file is part of Leela Chess Zero.
  Copyright (C) 2018 The LCZero Authors

  Leela Chess is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Leela Chess is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Leela Chess.  If not, see <http://www.gnu.org/licenses/>.

  Additional permission under GNU GPL version 3 section 7

  If you modify this Program, or any covered work, by linking or
  combining it with NVIDIA Corporation's libraries from the NVIDIA CUDA
  Toolkit and the NVIDIA CUDA Deep Neural Network library (or a
  modified version of those libraries), containing parts covered by the
  terms of the respective license agreement, the licensors of this
  Program grant you additional permission to convey the resulting work.
*/

#include "utils/logging.h"
#include <iomanip>
#include <thread>

namespace lczero {

namespace {
size_t kBufferSizeLines = 200;
}  // namespace

Logging& Logging::Get() {
  static Logging logging;
  return logging;
}

void Logging::WriteLineRaw(const std::string& line) {
  Mutex::Lock lock_(mutex_);
  if (!filename_.empty()) {
    file_ << line << std::endl;
  } else {
    buffer_.push_back(line);
    if (buffer_.size() > kBufferSizeLines) buffer_.pop_front();
  }
}

void Logging::SetFilename(const std::string& filename) {
  Mutex::Lock lock_(mutex_);
  if (filename_ == filename) return;
  filename_ = filename;
  if (filename.empty()) {
    file_.close();
    return;
  }
  file_.open(filename, std::ios_base::app);
  file_ << "\n\n============= Log started. =============" << std::endl;
  for (const auto& line : buffer_) file_ << line << std::endl;
  buffer_.clear();
}

LogMessage::LogMessage(const char* file, int line) {
  using namespace std::chrono;
  auto time_now = system_clock::now();
  auto ms =
      duration_cast<milliseconds>(time_now.time_since_epoch()).count() % 1000;
  auto timer = system_clock::to_time_t(time_now);
  *this << std::put_time(std::localtime(&timer), "%m%d %T") << '.'
        << std::setfill('0') << std::setw(3) << ms << ' ' << std::setfill(' ')
        << std::this_thread::get_id() << std::setfill('0') << ' ' << file << ':'
        << line << "] ";
}

LogMessage::~LogMessage() { Logging::Get().WriteLineRaw(str()); }

}  // namespace lczero