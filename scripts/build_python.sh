#!/bin/bash
# Build C++ library with Python bindings

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the shared library for Python
make db_python -j$(nproc)

echo ""
echo "✓ Library built successfully at: build/libdb_python.so"
echo ""
echo "To run the Python GUI:"
echo "  cd python && python3 gui.py"