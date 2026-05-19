#include "catalog.h"
#include "storage.h"
#include <filesystem>
#include <stdexcept>
#include <algorithm>

namespace fs = std::filesystem;

Catalog::Catalog(const std::string& root_path) : root_path_(root_path) {
    fs::create_directories(root_path);
}

void Catalog::loadAll() {
    for (const auto& entry : fs::directory_iterator(root_path_)) {
        if (entry.is_directory()) {
            std::string db_name = entry.path().filename().string();
            loadDatabase(entry.path().string(), db_name);
        }
    }
}

void Catalog::loadDatabase(const std::string& db_path, const std::string& db_name) {
    auto meta = DatabaseMetadata::load(db_name, db_path);
    metadata_.emplace(db_name, std::move(meta));

    // Загружаем таблицы
    const auto& tables_list = metadata_.at(db_name).tables();
    for (const auto& tbl_name : tables_list) {
        std::string tbl_file = db_path + "/" + tbl_name + ".json";
        if (fs::exists(tbl_file)) {
            Table tbl = Storage::load(tbl_file);
            databases_[db_name].emplace(tbl_name, std::move(tbl));
        }
    }
}

void Catalog::createDatabase(const std::string& db_name) {
    if (metadata_.count(db_name))
        throw std::runtime_error("Database already exists: " + db_name);
    std::string db_path = root_path_ + "/" + db_name;
    fs::create_directory(db_path);
    DatabaseMetadata meta(db_name, db_path);
    meta.save();
    metadata_.emplace(db_name, std::move(meta));
    databases_.emplace(db_name, std::map<std::string, Table>{});
}

void Catalog::dropDatabase(const std::string& db_name) {
    if (!metadata_.count(db_name))
        throw std::runtime_error("Database does not exist: " + db_name);
    std::string db_path = root_path_ + "/" + db_name;
    fs::remove_all(db_path);
    metadata_.erase(db_name);
    databases_.erase(db_name);
}

void Catalog::createTable(const std::string& db_name, const std::string& table_name,
                          const std::vector<std::string>& col_names,
                          const std::vector<ValueType>& col_types) {
    if (!metadata_.count(db_name))
        throw std::runtime_error("Database not found: " + db_name);
    if (metadata_.at(db_name).hasTable(table_name))
        throw std::runtime_error("Table already exists: " + table_name);

    Table new_tbl(table_name);
    new_tbl.setSchema(col_names, col_types);
    databases_.at(db_name).emplace(table_name, std::move(new_tbl));
    metadata_.at(db_name).addTable(table_name);

    // Сохраняем пустую таблицу
    std::string tbl_path = root_path_ + "/" + db_name + "/" + table_name + ".json";
    Storage::save(databases_.at(db_name).at(table_name), tbl_path);
}

void Catalog::dropTable(const std::string& db_name, const std::string& table_name) {
    if (!metadata_.count(db_name))
        throw std::runtime_error("Database not found: " + db_name);
    if (!metadata_.at(db_name).hasTable(table_name))
        throw std::runtime_error("Table not found: " + table_name);

    std::string tbl_path = root_path_ + "/" + db_name + "/" + table_name + ".json";
    fs::remove(tbl_path);
    databases_.at(db_name).erase(table_name);
    metadata_.at(db_name).removeTable(table_name);
}

Table& Catalog::getTable(const std::string& db_name, const std::string& table_name) {
    if (!metadata_.count(db_name))
        throw std::runtime_error("Database not found: " + db_name);
    if (!metadata_.at(db_name).hasTable(table_name))
        throw std::runtime_error("Table not found: " + table_name);
    return databases_.at(db_name).at(table_name);
}

void Catalog::saveAll() {
    for (const auto& [db_name, tables] : databases_) {
        saveDatabase(db_name);
    }
}

void Catalog::saveDatabase(const std::string& db_name) {
    std::string db_path = root_path_ + "/" + db_name;
    for (const auto& [tbl_name, tbl] : databases_.at(db_name)) {
        if (tbl.isDirty()) {
            std::string tbl_path = db_path + "/" + tbl_name + ".json";
            Storage::save(tbl, tbl_path);
        }
    }
    metadata_.at(db_name).save();
}

std::vector<std::string> Catalog::listDatabases() const {
    std::vector<std::string> dbs;
    for (const auto& [name, _] : metadata_) {
        dbs.push_back(name);
    }
    return dbs;
}