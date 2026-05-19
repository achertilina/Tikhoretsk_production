#include "result.h"
#include "../sql/result.h"

// Конструктор по умолчанию (приватный)
ClientResult::ClientResult() : is_error(false), is_select(false), affected(0) {}

// Конструктор из внутреннего QueryResult
ClientResult::ClientResult(const QueryResult& internal) {
    switch (internal.type) {
        case QueryResult::SELECT:
            is_select = true;
            cols = internal.column_names;
            for (const auto& row : internal.rows) {
                std::vector<std::string> str_row;
                for (const auto& val : row) str_row.push_back(val.toString());
                rows.push_back(str_row);
            }
            break;
        case QueryResult::AFFECTED:
            affected = internal.affected_rows;
            break;
        case QueryResult::ERROR:
            is_error = true;
            error_msg = internal.error_message;
            break;
    }
}

// Статический метод для создания ошибки
ClientResult ClientResult::makeError(const std::string& msg) {
    ClientResult res;
    res.is_error = true;
    res.error_msg = msg;
    return res;
}

std::vector<std::string> ClientResult::getRowAsStrings(size_t row_idx) const {
    if (row_idx >= rows.size()) return {};
    return rows[row_idx];
}