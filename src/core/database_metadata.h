#pragma once

#include <string>
#include <vector>

class DatabaseMetadata {
public:
    DatabaseMetadata(const std::string& db_name, const std::string& data_dir);
    void addTable(const std::string& table_name);
    void removeTable(const std::string& table_name);
    bool hasTable(const std::string& table_name) const;
    const std::vector<std::string>& tables() const { return table_names_; }
    void save() const;
    static DatabaseMetadata load(const std::string& db_name, const std::string& data_dir);

private:
    std::string db_name_;
    std::string metadata_file_path_;
    std::vector<std::string> table_names_;
};