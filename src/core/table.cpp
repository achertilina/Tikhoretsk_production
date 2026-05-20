#include "table.h"
#include <stdexcept>

Table::Table(const std::string& name) : name_(name) {}

void Table::setSchema(const std::vector<std::string>& col_names,
                      const std::vector<ValueType>& col_types) {
    if (col_names.size() != col_types.size())
        throw std::runtime_error("Column names and types size mismatch");
    col_names_ = col_names;
    col_types_ = col_types;
    dirty_ = true;
}

void Table::insertRow(const std::vector<Value>& row) {
    if (row.size() != col_names_.size())
        throw std::runtime_error("Row size does not match schema");
    rows_.push_back(row);
    dirty_ = true;
}

void Table::updateRow(size_t row_index, const std::unordered_map<std::string, Value>& updates) {
    if (row_index >= rows_.size()) throw std::runtime_error("Row index out of range");
    for (const auto& [col_name, new_val] : updates) {
        size_t col_idx = -1;
        for (size_t i = 0; i < col_names_.size(); ++i) {
            if (col_names_[i] == col_name) { col_idx = i; break; }
        }
        if (col_idx == -1) throw std::runtime_error("Unknown column: " + col_name);
        rows_[row_index][col_idx] = new_val;
    }
    dirty_ = true;
}

void Table::deleteRow(size_t row_index) {
    if (row_index >= rows_.size()) throw std::runtime_error("Row index out of range");
    rows_.erase(rows_.begin() + row_index);
    dirty_ = true;
}

std::vector<size_t> Table::filter(std::function<bool(const std::vector<Value>&)> predicate) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < rows_.size(); ++i) {
        if (predicate(rows_[i])) indices.push_back(i);
    }
    return indices;
}

Table Table::project(const std::vector<size_t>& col_indices) const {
    Table new_tbl(name_ + "_project");
    std::vector<std::string> new_names;
    std::vector<ValueType> new_types;
    for (size_t idx : col_indices) {
        new_names.push_back(col_names_[idx]);
        new_types.push_back(col_types_[idx]);
    }
    new_tbl.setSchema(new_names, new_types);
    for (const auto& row : rows_) {
        std::vector<Value> new_row;
        for (size_t idx : col_indices) new_row.push_back(row[idx]);
        new_tbl.insertRow(new_row);
    }
    return new_tbl;
}