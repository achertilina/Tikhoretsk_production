#pragma once

#include "table.h"
#include <string>

class Storage {
public:
    static void save(const Table& table, const std::string& filepath);
    static Table load(const std::string& filepath);
};