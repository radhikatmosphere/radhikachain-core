#!/usr/bin/env bash
# Dependencias para compilar radhikachain-core en Ubuntu/Debian.
set -euo pipefail

sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config curl make patch zip \
  libssl-dev libevent-dev \
  libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
  libboost-test-dev libboost-chrono-dev libboost-program-options-dev \
  libzmq3-dev libsqlite3-dev \
  libcapnp-dev capnproto \
  libnatpmp-dev libminiupnpc-dev

echo "Linux deps OK. Para Windows cross-compile también:"
echo "  sudo apt-get install -y g++-mingw-w64-x86-64-posix"