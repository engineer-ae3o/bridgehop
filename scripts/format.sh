#!/bin/bash
set -euo pipefail

cmake --preset tests-debug
cmake --build --preset tests-debug --target format
