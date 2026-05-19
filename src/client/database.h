#pragma once

#include <string>
#include <memory>

namespace db {

// Временная заглушка для Catalog
class Catalog {
public:
    void load_all() {}
    void save_all() {}
};

class Result;

class Database {
public:
    Database();
    ~Database();
    
    void open(const std::string& root_path);
    void close();
    Result execute(const std::string& sql);
    
private:
    std::unique_ptr<Catalog> catalog_;
};

} // namespace db
