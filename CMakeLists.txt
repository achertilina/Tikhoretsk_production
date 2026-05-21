cmake_minimum_required(VERSION 3.10)
project(TikhoretskDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем -fPIC для всех объектов
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Поддержка nlohmann/json
if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    find_path(NLOHMANN_INCLUDE_DIR nlohmann/json.hpp)
    if(NLOHMANN_INCLUDE_DIR)
        set(NLOHMANN_JSON_INCLUDE_DIR ${NLOHMANN_INCLUDE_DIR})
    else()
        find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp 
            PATHS /usr/include /usr/local/include
            PATH_SUFFIXES nlohmann)
        if(NOT NLOHMANN_JSON_INCLUDE_DIR)
            message(FATAL_ERROR "nlohmann/json.hpp not found")
        endif()
    endif()
endif()

message(STATUS "Using nlohmann/json from: ${NLOHMANN_JSON_INCLUDE_DIR}")

# ========================
# Статическая библиотека с -fPIC
# ========================
add_library(db_lib STATIC
    src/core/value.cpp
    src/core/table.cpp
    src/core/storage.cpp
    src/core/database_metadata.cpp
    src/core/catalog.cpp
    src/sql/ast.cpp
    src/sql/parser.cpp
    src/sql/executor.cpp
    src/sql/result.cpp
    src/client/database.cpp
    src/client/result.cpp
)

target_include_directories(db_lib PUBLIC src)
target_include_directories(db_lib PRIVATE ${NLOHMANN_JSON_INCLUDE_DIR})

# Принудительно добавляем -fPIC для статической библиотеки
set_property(TARGET db_lib PROPERTY POSITION_INDEPENDENT_CODE ON)

# ========================
# Консольный интерфейс
# ========================
add_executable(db_console
    src/console/main.cpp
)

target_link_libraries(db_console db_lib)
target_include_directories(db_console PRIVATE src)

# ========================
# Динамическая библиотека для Python
# ========================
add_library(db_python SHARED
    src/client/c_api.cpp
)

target_link_libraries(db_python db_lib)
target_include_directories(db_python PRIVATE src)

set_target_properties(db_python PROPERTIES
    PREFIX ""
    SUFFIX "${CMAKE_SHARED_LIBRARY_SUFFIX}"
)

if(UNIX AND NOT APPLE)
    set_target_properties(db_python PROPERTIES
        LINK_FLAGS "-Wl,-rpath,."
    )
endif()
