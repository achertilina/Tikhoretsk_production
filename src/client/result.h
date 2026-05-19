#pragma once

#include <string>
#include <vector>

namespace db {

/**
 * Результат выполнения SQL-запроса.
 * Может быть:
 * 1) SELECT — тогда есть колонки и строки
 * 2) INSERT/UPDATE/DELETE — тогда есть количество затронутых строк
 * 3) Ошибка — тогда есть сообщение об ошибке
 */
class Result {
public:
    // Статические методы для создания результата (используются бэкендом)
    static Result success_table(const std::vector<std::string>& columns,
                                 const std::vector<std::vector<std::string>>& rows);
    static Result success_affected(int rows);
    static Result error(const std::string& message);
    
    // Проверки
    bool success() const;      // Успешно ли выполнен запрос?
    bool is_select() const;    // Это SELECT?
    
    // Геттеры для данных
    std::string error_message() const;
    std::vector<std::string> get_column_names() const;
    std::vector<std::vector<std::string>> get_rows_as_strings() const;
    int get_affected_rows() const;
    size_t get_row_count() const;  // Количество строк в результате SELECT
    
private:
    Result();  // Приватный конструктор — создаем только через статические методы
    
    // Внутренние поля
    bool success_;
    bool select_;
    std::string error_msg_;
    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> rows_;
    int affected_rows_;
};

} // namespace db