#include "client/database.h"
#include "client/result.h"

namespace db {

Database::Database() : catalog_(std::make_unique<Catalog>()) {}

Database::~Database() {
    close();
}

void Database::open(const std::string& root_path) {
    // Временная заглушка
    (void)root_path;  // подавляем warning о неиспользуемом параметре
    if (catalog_) {
        catalog_->load_all();
    }
}

void Database::close() {
    if (catalog_) {
        catalog_->save_all();
    }
}

Result Database::execute(const std::string& sql) {
    // Временная заглушка для демо
    
    // SELECT
    if (sql.find("SELECT") == 0 || sql.find("select") == 0) {
        return Result::success_table(
            {"id", "name", "age"},
            {{"1", "Alice", "25"}, {"2", "Bob", "30"}, {"3", "Charlie", "35"}}
        );
    }
    
    // INSERT
    if (sql.find("INSERT") == 0 || sql.find("insert") == 0) {
        return Result::success_affected(1);
    }
    
    // UPDATE
    if (sql.find("UPDATE") == 0 || sql.find("update") == 0) {
        return Result::success_affected(1);
    }
    
    // DELETE
    if (sql.find("DELETE") == 0 || sql.find("delete") == 0) {
        return Result::success_affected(1);
    }
    
    // CREATE
    if (sql.find("CREATE") == 0 || sql.find("create") == 0) {
        return Result::success_affected(0);
    }
    
    // DROP
    if (sql.find("DROP") == 0 || sql.find("drop") == 0) {
        return Result::success_affected(0);
    }
    
    // Ошибка для непонятных запросов
    return Result::error("Unknown or unsupported query");
}

} // namespace db
