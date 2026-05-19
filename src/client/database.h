#pragma once

#include <string>
#include <memory>

namespace db {

// Forward declaration — скрываем внутренности
class Catalog;
class Result;

/**
 * Главный класс для работы с базой данных.
 * Это ФАСАД (паттерн Facade) — простой интерфейс к сложной системе.
 * 
 * Ты (фронтенд) используешь ТОЛЬКО этот класс.
 */
class Database {
public:
    Database();
    ~Database();
    
    /**
     * Открывает базу данных по пути root_path.
     * В этой папке будут лежать все базы данных.
     */
    void open(const std::string& root_path);
    
    /**
     * Выполняет SQL-запрос.
     * @param sql Строка с запросом (например, "SELECT * FROM users")
     * @return Result — объект с результатом или ошибкой
     */
    Result execute(const std::string& sql);
    
    /**
     * Закрывает базу данных, сохраняя все изменения.
     */
    void close();
    
private:
    std::unique_ptr<Catalog> catalog_;  // Внутренний менеджер данных
};

} // namespace db