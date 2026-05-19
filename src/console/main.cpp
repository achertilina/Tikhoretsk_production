#include "client/database.h"
#include "client/result.h"

#include <iostream>
#include <string>
#include <iomanip>   // для форматирования таблицы

using namespace db;

/**
 * Красиво выводит таблицу из результата SELECT
 */
void print_table(const Result& res) {
    if (!res.is_select()) return;
    
    auto columns = res.get_column_names();
    auto rows = res.get_rows_as_strings();
    
    // Выводим заголовки
    for (const auto& col : columns) {
        std::cout << std::setw(15) << std::left << col;
    }
    std::cout << std::endl;
    
    // Разделитель
    for (size_t i = 0; i < columns.size(); ++i) {
        std::cout << std::setw(15) << std::setfill('-') << "" << std::setfill(' ');
    }
    std::cout << std::endl;
    
    // Выводим строки
    for (const auto& row : rows) {
        for (const auto& value : row) {
            std::cout << std::setw(15) << std::left << value;
        }
        std::cout << std::endl;
    }
}

/**
 * Главная функция — точка входа в программу
 */
int main() {
    Database db;
    
    // Открываем базу данных (папка data будет создана автоматически)
    db.open("data/");
    
    std::cout << "========================================" << std::endl;
    std::cout << "   Tikhoretsk Production DB Console     " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  - Enter SQL query ending with ;" << std::endl;
    std::cout << "  - Type 'exit' to quit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::string line;
    std::string full_query;
    
    while (true) {
        std::cout << "db> ";
        full_query.clear();
        
        // Читаем многострочный запрос до символа ';'
        while (true) {
            if (!std::getline(std::cin, line)) {
                // Пользователь нажал Ctrl+D
                std::cout << std::endl;
                db.close();
                return 0;
            }
            
            // Выход из программы
            if (line == "exit") {
                db.close();
                return 0;
            }
            
            full_query += line + " ";
            
            // Если нашли точку с запятой — запрос готов к выполнению
            if (line.find(';') != std::string::npos) {
                break;
            }
        }
        
        // Выполняем запрос через API
        Result res = db.execute(full_query);
        
        // Обрабатываем результат
        if (!res.success()) {
            std::cout << "ERROR: " << res.error_message() << std::endl;
            continue;
        }
        
        if (res.is_select()) {
            print_table(res);
            std::cout << "-> " << res.get_row_count() << " row(s) returned." << std::endl;
        } else {
            std::cout << "-> " << res.get_affected_rows() << " row(s) affected." << std::endl;
        }
    }
    
    db.close();
    return 0;
}