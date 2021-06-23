# Copyright 2021 RISE Research Institute of Sweden. All rights reserved.
import time
import socket
import argparse
from pathlib import Path

HOST = "127.0.0.1"

LOG_FILE_PATH = Path(__file__).parent / "nmea2000_sample.log"

with LOG_FILE_PATH.open() as f:
    NMEA2000_FRAMES = f.readlines()

def main():
    parser = argparse.ArgumentParser(description="Fake NMEA2000 frame streamer over UDP")
    parser.add_argument("port", type=int, help="Port number to send to")

    args = parser.parse_args()

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    for frame in NMEA2000_FRAMES:
        mid = len(frame) // 2
        s.sendto(frame[:mid].encode(),(HOST, args.port))
        time.sleep(0.001)
        s.sendto(frame[mid:].encode(),(HOST, args.port))
        time.sleep(0.001)

if __name__ == "__main__":
    main()