#pragma once

#include <string>
#include <vector>

class ClientResult {
public:
    static ClientResult makeError(const std::string& msg);

    bool success() const { return !is_error; }
    bool isSelect() const { return is_select; }
    int affectedRows() const { return affected; }
    std::string errorMessage() const { return error_msg; }

    const std::vector<std::string>& columnNames() const { return cols; }
    std::vector<std::string> getRowAsStrings(size_t row_idx) const;
    size_t rowCount() const { return rows.size(); }

private:
    friend class Database;
    ClientResult();  // приватный конструктор по умолчанию
    ClientResult(const struct QueryResult& internal);

    bool is_error = false;
    bool is_select = false;
    int affected = 0;
    std::string error_msg;
    std::vector<std::string> cols;
    std::vector<std::vector<std::string>> rows;
};