#include "client/database.h"
#include "client/result.h"

namespace db {

Database::Database() : catalog_(nullptr) {}

Database::~Database() {
    close();
}

void Database::open(const std::string& root_path) {
    // TODO: бэкенд реализует
    // Пока заглушка
}

void Database::close() {
    // TODO: бэкенд реализует
    // Пока заглушка
}

Result Database::execute(const std::string& sql) {
    // TODO: бэкенд реализует
    // Пока возвращаем заглушку, чтобы твой фронтенд мог работать
    
    // Если запрос начинается с SELECT — возвращаем тестовую таблицу
    if (sql.find("SELECT") == 0 || sql.find("select") == 0) {
        return Result::success_table(
            {"id", "name", "age"},
            {{"1", "Alice", "25"}, {"2", "Bob", "30"}}
        );
    }
    
    // Если INSERT/UPDATE/DELETE
    if (sql.find("INSERT") == 0 || sql.find("UPDATE") == 0 || 
        sql.find("DELETE") == 0 || sql.find("insert") == 0 ||
        sql.find("update") == 0 || sql.find("delete") == 0) {
        return Result::success_affected(1);
    }
    
    // Остальное — ошибка
    return Result::error("Unknown query");
}

} // namespace db