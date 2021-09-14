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
#include <filesystem>
#include <iostream>
#include <optional>

#include "CLI/CLI.hpp"
#include "NMEA2000_assembler.hpp"
#include "cluon/Envelope.hpp"
#include "cluon/OD4Session.hpp"
#include "cluon/TCPConnection.hpp"
#include "cluon/UDPReceiver.hpp"
#include "morise-brefv.hpp"
#include "spdlog/sinks/daily_file_sink.h"

auto main(int argc, char **argv) -> int {
  CLI::App app{"NMEA2000 UDP Eavesdropper"};

  // General options for this service
  uint16_t cid = 111;
  app.add_option("-c,--cid", cid, "OpenDaVINCI session id");
  uint16_t id = 1;
  app.add_option("-i,--id", id, "Identification id of this microservice");
  std::filesystem::path dump_path{"NMEA2000/frames.txt"};
  app.add_option("-p,--path", dump_path,
                 "Absolute or relative path to a non-existing file where the "
                 "recorded NMEA2000 frames will be dumped to disk");
  bool verbose = false;
  app.add_flag("--verbose", verbose, "Print to cout");

  app.require_subcommand(1);

  // Gather subcommand
  auto gather = app.add_subcommand(
      "gather",
      "Run in data gathering mode, i.e connect to a UDP stream, listen for "
      "incoming NMEA2000 frames and publish them on an OD4 session. "
      "Optionally, it is possible to dump the incoming NMEA2000 frames "
      "directly to disk");
  std::string address;
  gather->add_option("-a,--address", address, "IP address of stream")
      ->required();
  uint16_t port;
  gather->add_option("-p,--port", port, "Port number to connect to")
      ->required();
  bool is_UDP = false;
  gather->add_flag("--udp", is_UDP,
                   "If application should use UDP to connect, default is TCP");
  bool standalone = false;
  gather->add_flag("--standalone", standalone,
                   "If application should be run in standalone mode, i.e. "
                   "dumping any incoming NMEA2000 frames directly to disk "
                   "instead of publishing to an OD4 session");
  gather->callback([&]() {
    cluon::OD4Session od4{cid};
    std::optional<std::shared_ptr<spdlog::logger>> sink = std::nullopt;

    if (standalone) {
      sink = spdlog::daily_logger_mt("daily_logger", dump_path.string(), 0, 0);
      (*sink)->set_pattern("%v");
    }

    // Wrap everything in a frame handler lambda
    auto sentence_handler =
        [&od4, &id, &sink, &verbose](
            const Frame &frame,
            const std::chrono::system_clock::time_point &tp) {
          auto timestamp = cluon::time::convert(tp);

          // Handle
          auto [header, data] = frame;

          if (!sink.has_value()) {
            // Publish to OD4 session
            morise::raw::NMEA2000 m;
            m.header(header);
            m.data(data);
            od4.send(m, timestamp, id);
          } else {
            // Log to disk
            std::stringstream message;
            message << cluon::time::toMicroseconds(timestamp) << " " << header
                    << " " << data;
            (*sink)->info(message.str());
            (*sink)->flush();
          }

          if (verbose) {
            std::cout << header << " " << data << std::endl;
          }
        };

    // And finally use this in a FrameAssembler
    NMEA2000FrameAssembler assembler(sentence_handler);

    if (is_UDP) {
      // Setup a connection to an UDP source with incoming NMEA2000
      // messages
      cluon::UDPReceiver connection{
          address, port,
          [&assembler](std::string &&d, std::string && /*from*/,
                       std::chrono::system_clock::time_point &&tp) noexcept {
            assembler(d, std::move(tp));
          }};

      using namespace std::literals::chrono_literals;  // NOLINT
      while (connection.isRunning()) {
        std::this_thread::sleep_for(1s);
      }

    } else {
      cluon::TCPConnection connection{
          address, port,
          [&assembler](std::string &&d,
                       std::chrono::system_clock::time_point &&tp) noexcept {
            assembler(d, std::move(tp));
          },
          [&argv]() {
            std::cerr << "[" << argv[0] << "] Connection lost." << std::endl;
            exit(1);
          }};

      // Just sleep as this microservice is data driven.
      using namespace std::literals::chrono_literals;  // NOLINT
      while (connection.isRunning()) {
        std::this_thread::sleep_for(1s);
      }
    }
  });

  // Log subcommand
  auto log = app.add_subcommand(
      "log",
      "Run in data logging mode, i.e. connect to an OD4 session, listen for "
      "all raw NMEA2000 frames flying by, published by nodes in 'gather "
      "mode' and dump these to disk");
  log->callback([&]() {
    // A sink that rotates at midnight
    auto sink =
        spdlog::daily_logger_mt("daily_logger", dump_path.string(), 0, 0);
    sink->set_pattern("%v");

    // Setup a cluon instance
    cluon::OD4Session od4{cid};
    od4.dataTrigger(
        morise::raw::NMEA2000::ID(), [&](cluon::data::Envelope &&envelope) {
          auto msg =
              cluon::extractMessage<morise::raw::NMEA2000>(std::move(envelope));
          std::stringstream record;
          record << cluon::time::toMicroseconds(envelope.sampleTimeStamp())
                 << " " << envelope.senderStamp() << " " << msg.header() << " "
                 << msg.data();
          sink->info(record.str());
          sink->flush();

          if (verbose) {
            std::cout << record.str() << std::endl;
          }
        });

    using namespace std::literals::chrono_literals;  // NOLINT
    while (od4.isRunning()) {
      std::this_thread::sleep_for(1s);
    }
  });

  CLI11_PARSE(app, argc, argv);

  return 0;
}
