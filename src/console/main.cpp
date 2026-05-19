#include "client/database.h"
#include "client/result.h"

#include <iostream>
#include <string>
#include <iomanip>

/**
 * Красиво выводит таблицу из результата SELECT
 */
void print_table(const ClientResult& res) {
    if (!res.isSelect()) return;
    
    auto columns = res.columnNames();
    
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
    for (size_t row_idx = 0; row_idx < res.rowCount(); ++row_idx) {
        auto row = res.getRowAsStrings(row_idx);
        for (const auto& value : row) {
            std::cout << std::setw(15) << std::left << value;
        }
        std::cout << std::endl;
    }
}

int main() {
    Database db;
    
    if (!db.open("data/")) {
        std::cerr << "Failed to open storage" << std::endl;
        return 1;
    }
    
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
                std::cout << std::endl;
                db.close();
                return 0;
            }
            
            if (line == "exit") {
                db.close();
                return 0;
            }
            
            full_query += line + " ";
            
            if (line.find(';') != std::string::npos) {
                break;
            }
        }
        
        ClientResult res = db.execute(full_query);
        
        if (!res.success()) {
            std::cout << "ERROR: " << res.errorMessage() << std::endl;
            continue;
        }
        
        if (res.isSelect()) {
            print_table(res);
            std::cout << "-> " << res.rowCount() << " row(s) returned." << std::endl;
        } else {
            std::cout << "-> " << res.affectedRows() << " row(s) affected." << std::endl;
        }
    }
    
    db.close();
    return 0;
}