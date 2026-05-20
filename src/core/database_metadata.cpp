#include "database_metadata.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>

using json = nlohmann::json;

DatabaseMetadata::DatabaseMetadata(const std::string& db_name, const std::string& data_dir)
    : db_name_(db_name) {
    std::filesystem::create_directories(data_dir);
    metadata_file_path_ = data_dir + "/.metadata.json";
}

void DatabaseMetadata::addTable(const std::string& table_name) {
    for (const auto& t : table_names_)
        if (t == table_name) return;
    table_names_.push_back(table_name);
    save();
}

void DatabaseMetadata::removeTable(const std::string& table_name) {
    auto it = std::find(table_names_.begin(), table_names_.end(), table_name);
    if (it != table_names_.end()) {
        table_names_.erase(it);
        save();
    }
}

bool DatabaseMetadata::hasTable(const std::string& table_name) const {
    return std::find(table_names_.begin(), table_names_.end(), table_name) != table_names_.end();
}

void DatabaseMetadata::save() const {
    json j;
    j["database_name"] = db_name_;
    j["tables"] = table_names_;
    std::ofstream ofs(metadata_file_path_);
    if (!ofs) throw std::runtime_error("Cannot save metadata for " + db_name_);
    ofs << j.dump(4);
}

DatabaseMetadata DatabaseMetadata::load(const std::string& db_name, const std::string& data_dir) {
    std::string filepath = data_dir + "/.metadata.json";
    if (!std::filesystem::exists(filepath)) {
        // new database
        return DatabaseMetadata(db_name, data_dir);
    }
    std::ifstream ifs(filepath);
    if (!ifs) throw std::runtime_error("Cannot open metadata for " + db_name);
    json j;
    ifs >> j;
    DatabaseMetadata meta(db_name, data_dir);
    meta.table_names_ = j["tables"].get<std::vector<std::string>>();
    return meta;
}