#!/bin/bash

# Цвета
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Параметры по умолчанию
HOST=${1:-localhost}
PORT=${2:-5432}

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}   TikhoretskDB Launcher${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${YELLOW}🌐 Host: $HOST${NC}"
echo -e "${YELLOW}🔌 Port: $PORT${NC}"
echo -e "${GREEN}========================================${NC}"

# Проверяем, существует ли исполняемый файл
if [ ! -f "./build/db_console" ]; then
    echo -e "${YELLOW}⚠️  Executable not found. Running build.sh...${NC}"
    ./build.sh
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ Build failed. Cannot start database.${NC}"
        exit 1
    fi
fi

# Проверяем ещё раз после сборки
if [ ! -f "./build/db_console" ]; then
    echo -e "${RED}❌ Executable still not found. Build may have failed.${NC}"
    exit 1
fi

echo -e "${GREEN}🚀 Starting TikhoretskDB on $HOST:$PORT${NC}"
echo -e "${GREEN}📝 Type SQL queries ending with ';'${NC}"
echo -e "${GREEN}🚪 Type 'exit' to quit${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Запускаем БД
./build/db_console