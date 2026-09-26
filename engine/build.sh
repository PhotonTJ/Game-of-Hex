#!/usr/bin/env bash
# Builds the C++ engine (static lib + doctest suite + benchmark tool +
# pybind11 module), runs the test suite, and drops the compiled extension
# module next to the FastAPI app so `from . import hex_engine` resolves at
# import time. Used both for local development and the backend Dockerfile.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DHEX_BUILD_TESTS=ON \
  -DHEX_BUILD_TOOLS=OFF \
  -DHEX_BUILD_PYTHON_BINDINGS=ON

cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "== Running C++ unit tests =="
"${BUILD_DIR}/engine/hex_engine_tests"

MODULE=$(find "${BUILD_DIR}/engine/bindings" -maxdepth 1 -name "hex_engine*.so" | head -n1)
if [[ -z "${MODULE}" ]]; then
  echo "error: built pybind11 module not found" >&2
  exit 1
fi

mkdir -p "${ROOT_DIR}/backend/app"
cp "${MODULE}" "${ROOT_DIR}/backend/app/"
echo "== Installed $(basename "${MODULE}") into backend/app/ =="
