# cluon-nmea2000

Copyright 2021 RISE Research Institute of Sweden - Maritime Operations. Licensed under the Apache License Version 2.0. For details, please contact Fredrik Olsson (fredrik.x.olsson(at)ri.se).

A [libcluon](https://github.com/chrberger/libcluon)-based microservice for eavesdropping on a NMEA2000 stream over either UDP or TCP. This software does not perform any parsing of the NMEA frames, merely assembles any fragmented messages into full frames. It can be run in two modes:
* `gather`, connect to a stream of incoming NMEA2000 frames and either:
  * publish to an OD4 session, or
  * log directly to disk (`--standalone`)
* `log`, listen to an OD4 session for raw NMEA2000 frames from other `gatherers` and dump these to an aggregated log file on disk

## How do I get it?
Each release of `cluon-nmea2000` is published as a docker image [here](https://github.com/orgs/MO-RISE/packages/container/package/cluon-nmea2000) and is publicly available.

Can also be used as a standalone commandline tool. No pre-built binaries are, however, provided for this purpose.

## Example docker-compose setup
The example below showcases a setup with two gatherers (listening on two separate stream (one UDP and one TCP)) and one logger that aggregates published messages from the gatherers into a single file.
```yaml
version: '2'
services:    
    gatherer_1:
        container_name: cluon-nmea2000-gatherer-1
        image: ghcr.io/mo-rise/cluon-nmea2000:v0.1.0
        restart: on-failure
        network_mode: "host"
        command: "--cid 111 --id 1 gather --udp -a 255.255.255.255 -p 1457"
    gatherer_2:
        container_name: cluon-nmea2000-gatherer-2
        image: ghcr.io/mo-rise/cluon-nmea2000:v0.1.0
        restart: on-failure
        network_mode: "host"
        command: "--cid 111 --id 2 gather -a 171.31.16.42 -p 6002"
    logger:
        container_name: cluon-nmea2000-logger
        image: ghcr.io/mo-rise/cluon-nmea2000:v0.1.0
        restart: on-failure
        network_mode: "host"
        volumes:
        - .:/opt/cluon-nmea2000
        command: "--cid 111 --path /opt/cluon-nmea2000/recordings/nmea2000.log log"
```

## Details

### Message set
This microservice introduce a new OD4-compatible message type (`raw.NMEA2000`) in the `risemo` message set. See https://github.com/MO-RISE/risemo-message-set for details.

### Build from source
This repository makes use of [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) for dependency resolution as an interal part of the CMake setup. As a result, the only requirements for building from source are:
* a C++17 compliant compiler
* CMake (>=3.14)

As part of the CMake configuration step, the following dependencies are downloaded and configured:
* [libcluon](https://github.com/chrberger/libcluon)
* [CLI11](https://github.com/CLIUtils/CLI11)
* [spdlog](https://github.com/gabime/spdlog)
* [doctest](https://github.com/onqtam/doctest) (for testing only)

To build (from the root directory of this repo):
```cmd
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j 8
```

### Tests

Unit tests for the NMEA01843 sentence assembler is compiled into the executable `cluon-nmea2000-tester`.

To run tests (after successful build):
```cmd
cd build
ctest -C Debug -T test --output-on-failure --
```

A simple integration test can also be done by running the compiled commandline tool (cluon-nmea2000) in conjunction with the provided python script `fake_nmea_stream_udp.py` in the `tests` folder.

### Development setup
This repo contains some configuration files (in the `.vscode`-folder) for getting started easy on the following setup:
* Ubuntu 20.04 (WSL2 is fine)
* GCC 9
* python 3
* VSCode as IDE, using the following extensions:
  - C/C++ (ms-vscode.cpptools)
  - C/C++ Extension Pack (ms-vscode.cpptools-extension-pack)
  - CMake Tools (ms-vscode.cmake-tools)
  - Python (ms-python.python)

Do the following steps to get started:
* Clone repo
* Create a python virtual environment (`python3 -m venv venv`) in the root of the repo.
* Open vscode in the repo root (`code .`)

The provided configuration is very lightweight and should be easily adaptable to other enviroments/setups.

