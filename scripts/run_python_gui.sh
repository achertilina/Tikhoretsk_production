#!/bin/bash
# Run Python GUI

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# Check if library exists
if [ ! -f "build/libdb_python.so" ]; then
    echo "Library not found. Building first..."
    ./scripts/build_python.sh
fi

# Run GUI
cd python
python3 gui.py