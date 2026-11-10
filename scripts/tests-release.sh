#!/bin/bash
set -euo pipefail

cmake --preset tests-release
cmake --build --preset tests-release
./build/tests-release/bridgehop.elf
