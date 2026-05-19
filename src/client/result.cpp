#include "client/result.h"

namespace db {

Result::Result()
    : success_(false)
    , select_(false)
    , affected_rows_(0) {}

Result Result::success_table(const std::vector<std::string>& columns,
                              const std::vector<std::vector<std::string>>& rows) {
    Result res;
    res.success_ = true;
    res.select_ = true;
    res.columns_ = columns;
    res.rows_ = rows;
    return res;
}

Result Result::success_affected(int rows) {
    Result res;
    res.success_ = true;
    res.select_ = false;
    res.affected_rows_ = rows;
    return res;
}

Result Result::error(const std::string& message) {
    Result res;
    res.success_ = false;
    res.error_msg_ = message;
    return res;
}

bool Result::success() const {
    return success_;
}

bool Result::is_select() const {
    return select_;
}

std::string Result::error_message() const {
    return error_msg_;
}

std::vector<std::string> Result::get_column_names() const {
    return columns_;
}

std::vector<std::vector<std::string>> Result::get_rows_as_strings() const {
    return rows_;
}

int Result::get_affected_rows() const {
    return affected_rows_;
}

size_t Result::get_row_count() const {
    return rows_.size();
}

} // namespace db