#!/bin/sh

set -eu

ROOT=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
IMAGE=${KOBUILD_QT5_IMAGE:-ghcr.io/nicoverbruggen/kobuild:qt5-v0.1.0}

if [ -n "${CONTAINER_ENGINE:-}" ]; then
  RUNTIME=$CONTAINER_ENGINE
elif command -v docker >/dev/null 2>&1; then
  RUNTIME=docker
elif command -v container >/dev/null 2>&1; then
  RUNTIME=container
  container system start
else
  echo "Error: Docker or Apple's container CLI is required." >&2
  exit 1
fi

"$RUNTIME" run --rm --user "$(id -u):$(id -g)" \
  --volume "$ROOT:/work" \
  --workdir /work \
  "$IMAGE" \
  sh -ec '
    cmake -S . -B build/qt5 -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=/tc/qt5/toolchain.cmake
    cmake --build build/qt5 --target koboroot
  '
