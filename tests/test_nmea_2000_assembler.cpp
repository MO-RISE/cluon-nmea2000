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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>  // NOLINT
#include <string>
#include <vector>

#include "YDEN-02_extractor.hpp"

TEST_CASE("Test NMEA2000FrameAssembler with empty payload.") {
  auto data = std::string();
  auto output = extract_N2k_frame(data);
  REQUIRE(output.empty());
}

TEST_CASE("Test NMEA2000FrameAssembler with payload of wrong format") {
  std::string data{"skfvdalfdadnsldasdc\nsakhbsacdsad\nahsdvds\nadjd"};

  auto output = extract_N2k_frame(data);
  REQUIRE(output.empty());
}

TEST_CASE("Test NMEA2000FrameAssembler with single correct sentence") {
  std::string data{"04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF\r\n"};

  auto output = extract_N2k_frame(data);

  REQUIRE_EQ(output, "15FD080E 3600029F73FFFFFF\r\n");
}
