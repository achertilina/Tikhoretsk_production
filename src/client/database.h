#pragma once

#include "result.h"
#include <memory>
#include <string>
#include <vector>

class Database {
public:
    Database();
    ~Database();

    bool open(const std::string& root_path);
    ClientResult execute(const std::string& sql);
    void close();

    // Переключение на указанную базу данных
    bool useDatabase(const std::string& db_name);

    // Получение списка всех баз данных
    std::vector<std::string> listDatabases() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};