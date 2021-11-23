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
#ifndef SRC_YDEN_02_EXTRACTOR_HPP_
#define SRC_YDEN_02_EXTRACTOR_HPP_

#include <chrono>  // NOLINT
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <utility>

#include "cluon/stringtoolbox.hpp"

inline std::string extract_N2k_frame(std::string &data) {  // NOLINT
  auto line = stringtoolbox::trim(data);
  auto parts = stringtoolbox::split(line, ' ');

  // Simple validation
  // Example dataset
  // 04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF
  // 04:54:52.151 R 15FD070E 36 C2 9F 73 FF 7F FB 03
  // 04:54:52.179 R 09F1120B B1 07 F2 00 00 FF 7F FD
  if (parts.size() != 11) {
    return std::string();
  }
  std::string identifier = parts[2];
  std::string payload;
  for (auto it = parts.begin() + 3; it != parts.end(); ++it) {
    payload += *it;
  }
  return identifier + " " + payload;
}

#endif  // SRC_YDEN_02_EXTRACTOR_HPP_ "
