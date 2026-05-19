cmake_minimum_required(VERSION 3.10)
project(TikhoretskDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================
# Статическая библиотека (ядро + API)
# ========================
add_library(db_lib STATIC
    # Core
    src/core/value.cpp
    src/core/table.cpp
    src/core/storage.cpp
    src/core/database_metadata.cpp
    src/core/catalog.cpp
    # SQL
    src/sql/ast.cpp
    src/sql/parser.cpp
    src/sql/executor.cpp
    src/sql/result.cpp
    # Client (API)
    src/client/database.cpp
    src/client/result.cpp
)

# Говорим, где искать заголовочные файлы
target_include_directories(db_lib PUBLIC src)

# ========================
# Консольный интерфейс (твой фронтенд)
# ========================
add_executable(db_console
    src/console/main.cpp
)

target_link_libraries(db_console db_lib)
target_include_directories(db_console PRIVATE src)

# ========================
# GUI (опционально, только если нужен)
# ========================
option(BUILD_GUI "Build GUI version" OFF)

if(BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    
    add_executable(db_gui
        src/gui/main.cpp
        src/gui/main_window.cpp
        src/gui/query_editor.cpp
        src/gui/results_table.cpp
        src/gui/connection_panel.cpp
    )
    
    target_link_libraries(db_gui Qt6::Widgets db_lib)
    target_include_directories(db_gui PRIVATE src)
endif()

# ========================
# Настройки для отладки
# ========================
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")