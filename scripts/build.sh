#!/bin/bash

# Установка cmake, если нужно (на macOS через Homebrew)
if [[ "$OSTYPE" == "darwin"* ]]; then
    if ! command -v cmake &> /dev/null; then
        echo "Installing cmake via Homebrew..."
        brew install cmake
    fi
fi

# Скачиваем nlohmann/json.hpp
mkdir -p libs
if [ ! -f "libs/nlohmann/json.hpp" ]; then
    echo "Downloading nlohmann/json.hpp..."
    mkdir -p libs/nlohmann
    curl -L https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp -o libs/nlohmann/json.hpp
fi

# Сборка
mkdir -p build
cd build

# Определяем количество ядер для macOS/Linux
if [[ "$OSTYPE" == "darwin"* ]]; then
    NPROC=$(sysctl -n hw.ncpu)
else
    NPROC=$(nproc)
fi

# Передаём путь к nlohmann/json
cmake .. -DNLOHMANN_JSON_INCLUDE_DIR="${PWD}/../libs"
make -j${NPROC}

echo ""
echo "✅ Build complete!"
echo "   Run ./build/db_console to start"