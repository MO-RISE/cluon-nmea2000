///  Copyright 2021 RISE Research Institute of Sweden - Maritime Operations
///
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///
///      http://www.apache.org/licenses/LICENSE-2.0
///
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
#ifndef SRC_NMEA2000_ASSEMBLER_HPP_
#define SRC_NMEA2000_ASSEMBLER_HPP_

#include <chrono>  // NOLINT
#include <functional>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

#include "cluon/stringtoolbox.hpp"

using Frame = std::pair<std::string, std::string>;

class NMEA2000FrameAssembler {
  std::string remainder_;
  std::function<void(const Frame &,
                     const std::chrono::system_clock::time_point &)>
      delegate_{};

 public:
  NMEA2000FrameAssembler(
      std::function<void(const Frame &,
                         const std::chrono::system_clock::time_point &)>
          delegate)
      : delegate_(delegate) {}

  void operator()(const std::string &data,
                  std::chrono::system_clock::time_point &&tp) {
    // Get total buffered data
    std::stringstream buffer{remainder_ + data};

    const std::chrono::system_clock::time_point time_stamp{std::move(tp)};

    // Handle any fully received lines
    std::string line;
    std::smatch match;
    while (std::getline(buffer, line, '\n')) {
      if (buffer.eof()) {
        // If we run out of buffer, we dont have a full line and bail
        // accordingly
        break;
      }

      // Extract identifier and data
      // Trim for any trailing whitespace
      line.erase(line.find_last_not_of(" \n\r\t") + 1);
      auto parts = stringtoolbox::split(line, ' ');

      // Simple validation
      // Example dataset
      // 04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF
      // 04:54:52.151 R 15FD070E 36 C2 9F 73 FF 7F FB 03
      // 04:54:52.179 R 09F1120B B1 07 F2 00 00 FF 7F FD
      if (parts.size() == 11) {
        std::string identifier = parts[2];
        std::string data;
        for (auto it = parts.begin() + 3; it != parts.end(); ++it) {
          data += *it;
        }
        delegate_(std::make_pair(identifier, data), time_stamp);
      }
    }
    // Make sure we save any remaining characters for next incoming
    remainder_ = line;
  }
};

#endif  // SRC_NMEA2000_ASSEMBLER_HPP_"
