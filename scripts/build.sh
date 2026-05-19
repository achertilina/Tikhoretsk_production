#!/bin/bash

# Цвета для красивого вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}   TikhoretskDB Build System${NC}"
echo -e "${BLUE}========================================${NC}"

# Определяем операционную систему
OS=$(uname -s)
echo -e "${BLUE}📋 Detected OS: $OS${NC}"

# Функция для установки пакетов на Linux
install_linux_deps() {
    if command -v apt &> /dev/null; then
        echo -e "${BLUE}📦 Using apt package manager${NC}"
        sudo apt update
        sudo apt install -y build-essential cmake g++
    elif command -v pacman &> /dev/null; then
        echo -e "${BLUE}📦 Using pacman package manager${NC}"
        sudo pacman -S --noconfirm base-devel cmake gcc
    elif command -v dnf &> /dev/null; then
        echo -e "${BLUE}📦 Using dnf package manager${NC}"
        sudo dnf install -y gcc-c++ cmake make
    else
        echo -e "${RED}❌ No known package manager found${NC}"
        echo -e "${RED}Please install manually: cmake, g++/clang++${NC}"
        exit 1
    fi
}

# Устанавливаем зависимости в зависимости от ОС
case "$OS" in
    Linux)
        install_linux_deps
        ;;
    Darwin)
        echo -e "${BLUE}📦 macOS detected, using Homebrew${NC}"
        if ! command -v brew &> /dev/null; then
            echo -e "${RED}❌ Homebrew not found. Install from https://brew.sh/${NC}"
            exit 1
        fi
        brew install cmake
        ;;
    MINGW*|CYGWIN*|MSYS*)
        echo -e "${BLUE}📦 Windows detected${NC}"
        echo -e "${RED}⚠️  Please install manually:${NC}"
        echo -e "  - CMake from https://cmake.org/download/"
        echo -e "  - MinGW or Visual Studio"
        exit 1
        ;;
    *)
        echo -e "${RED}❌ Unsupported OS: $OS${NC}"
        exit 1
        ;;
esac

# Проверяем наличие CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}❌ CMake not found after installation${NC}"
    exit 1
fi

echo -e "${GREEN}✅ All dependencies installed${NC}"

# Создаём папку для сборки
echo -e "${BLUE}🔨 Building project...${NC}"
mkdir -p build
cd build

# Запускаем CMake
echo -e "${BLUE}📋 Running CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ CMake configuration failed${NC}"
    exit 1
fi

# Собираем проект
echo -e "${BLUE}🔨 Compiling...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Build failed${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Build complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}📁 Build directory: build/${NC}"
echo -e "${GREEN}📚 Library: build/libdb_lib.a${NC}"
echo -e "${GREEN}🎮 Console: build/db_console${NC}"
echo -e "${GREEN}========================================${NC}"