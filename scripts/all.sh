#!/bin/bash
set -euo pipefail

cmake --preset debug
cmake --preset release
cmake --preset tests-debug
cmake --preset tests-release

cmake --build --preset debug
cmake --build --preset release
cmake --build --preset tests-debug
cmake --build --preset tests-release
