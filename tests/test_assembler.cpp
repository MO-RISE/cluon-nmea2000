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

#include "assembler.hpp"

TEST_CASE("Test Assembler with empty payload.") {
  std::string output;

  std::string DATA;

  Assembler a{[&output](std::string&& line,
                        std::chrono::system_clock::time_point&& timestamp) {
    output = std::move(line);
  }};
  a(std::move(DATA), std::chrono::system_clock::time_point());

  REQUIRE(output.empty());
}

TEST_CASE("Test Assembler with payload of wrong format") {
  size_t call_count{0};
  std::string output;

  std::string DATA{"skfvdalfdadnsldasdc\nsakhbsacdsad\nahsdvds\nadjd"};

  Assembler a{[&output, &call_count](
                  std::string&& frame,
                  std::chrono::system_clock::time_point&& timestamp) {
    output = std::move(frame);
    call_count++;
  }};
  a(std::move(DATA), std::chrono::system_clock::time_point());

  REQUIRE(output.empty());
  REQUIRE_EQ(call_count, 0);
}

TEST_CASE("Test Assembler with single correct sentence") {
  size_t call_count{0};
  std::string output;

  std::string DATA{"04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF\r\n"};

  Assembler a{[&output, &call_count](
                  std::string&& frame,
                  std::chrono::system_clock::time_point&& timestamp) {
    output = std::move(frame);
    call_count++;
  }};
  a(std::move(DATA), std::chrono::system_clock::time_point());

  REQUIRE_EQ(output, "15FD080E 3600029F73FFFFFF");
  REQUIRE_EQ(call_count, 1);
}

TEST_CASE(
    "Test Assembler with single correct sentence inbetween "
    "scramble") {
  size_t call_count{0};
  std::string output;

  std::string DATA{"04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF\r\n"};

  std::string scramble{"dadnsldasdc\n"};

  Assembler a{[&output, &call_count](
                  std::string&& frame,
                  std::chrono::system_clock::time_point&& timestamp) {
    output = std::move(frame);
    call_count++;
  }};
  a(scramble + DATA + scramble, std::chrono::system_clock::time_point());

  REQUIRE_EQ(output, "15FD080E 3600029F73FFFFFF");
  REQUIRE_EQ(call_count, 1);
}

TEST_CASE("Test Assembler with multiple sentences") {
  size_t call_count{0};
  std::vector<std::string> output;

  std::string DATA1{"04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF\r\n"};
  std::string DATA2{"04:54:52.151 R 15FD070E 36 C2 9F 73 FF 7F FB 03\r\n"};
  std::string DATA3{"04:54:52.179 R 09F1120B B1 07 F2 00 00 FF 7F FD\r\n"};

  Assembler a{[&output, &call_count](
                  std::string&& frame,
                  std::chrono::system_clock::time_point&& timestamp) {
    output.push_back(frame);
    call_count++;
  }};
  a(std::move(DATA1), std::chrono::system_clock::time_point());
  a(std::move(DATA2), std::chrono::system_clock::time_point());
  a(std::move(DATA3), std::chrono::system_clock::time_point());

  REQUIRE_EQ(call_count, 3);
  REQUIRE_EQ(output[0], "15FD080E 3600029F73FFFFFF");
  REQUIRE_EQ(output[1], "15FD070E 36C29F73FF7FFB03");
  REQUIRE_EQ(output[2], "09F1120B B107F20000FF7FFD");
}

TEST_CASE("Test Assembler with split sentence") {
  size_t call_count{0};
  std::vector<std::string> output;

  std::string DATA{"04:54:52.150 R 15FD080E 36 00 02 9F 73 FF FF FF\r\n"};

  std::string input =
      DATA.substr(13, DATA.size()) + DATA + DATA + DATA.substr(0, 17);

  Assembler a{[&output, &call_count](
                  std::string&& frame,
                  std::chrono::system_clock::time_point&& timestamp) {
    output.push_back(frame);
    call_count++;
  }};
  a(std::move(input), std::chrono::system_clock::time_point());

  REQUIRE_EQ(call_count, 2);
  REQUIRE_EQ(output[0], "15FD080E 3600029F73FFFFFF");
  REQUIRE_EQ(output[1], "15FD080E 3600029F73FFFFFF");
}
