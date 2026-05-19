#!/bin/bash

# Создаем папку для сборки
mkdir -p build
cd build

# Запускаем CMake и сборку
cmake ..
make -j$(nproc)

echo ""
echo "✅ Build complete!"
echo "   Run ./build/db_console to start"