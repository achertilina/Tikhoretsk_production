#pragma once

#include "table.h"
#include "database_metadata.h"
#include <map>
#include <string>
#include <memory>
#include <vector>

class Catalog {
public:
    explicit Catalog(const std::string& root_path);
    void loadAll();

    void createDatabase(const std::string& db_name);
    void dropDatabase(const std::string& db_name);
    void createTable(const std::string& db_name, const std::string& table_name,
                     const std::vector<std::string>& col_names,
                     const std::vector<ValueType>& col_types);
    void dropTable(const std::string& db_name, const std::string& table_name);
    Table& getTable(const std::string& db_name, const std::string& table_name);

    void saveAll();

    // Получить список всех баз данных
    std::vector<std::string> listDatabases() const;

private:
    std::string root_path_;
    std::map<std::string, std::map<std::string, Table>> databases_; // db -> table
    std::map<std::string, DatabaseMetadata> metadata_;

    void loadDatabase(const std::string& db_path, const std::string& db_name);
    void saveDatabase(const std::string& db_name);
};