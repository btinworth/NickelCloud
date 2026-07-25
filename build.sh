#!/bin/sh

set -e

cd "$(dirname "$0")"

if command -v docker >/dev/null 2>&1; then
  RUNTIME=docker
elif command -v container >/dev/null 2>&1; then
  RUNTIME=container
  container system start
else
  echo "Error: Docker or Apple's container CLI is required." >&2
  exit 1
fi

"$RUNTIME" run --rm --platform linux/amd64 \
  --volume "$PWD:$PWD" \
  --workdir "$PWD" \
  ghcr.io/pgaskin/nickeltc:1 \
  sh -c 'make clean && make && make koboroot'
