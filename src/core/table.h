#pragma once

#include "value.h"
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

class Table {
public:
    explicit Table(const std::string& name);

    void setSchema(const std::vector<std::string>& col_names,
                   const std::vector<ValueType>& col_types);
    void insertRow(const std::vector<Value>& row);
    void updateRow(size_t row_index, const std::unordered_map<std::string, Value>& updates);
    void deleteRow(size_t row_index);
    std::vector<size_t> filter(std::function<bool(const std::vector<Value>&)> predicate) const;
    Table project(const std::vector<size_t>& col_indices) const;

    // Getters
    const std::string& name() const { return name_; }
    const std::vector<std::string>& columnNames() const { return col_names_; }
    const std::vector<ValueType>& columnTypes() const { return col_types_; }
    const std::vector<std::vector<Value>>& rows() const { return rows_; }
    size_t rowCount() const { return rows_.size(); }
    bool isDirty() const { return dirty_; }
    void setDirty(bool dirty) { dirty_ = dirty; }

private:
    std::string name_;
    std::vector<std::string> col_names_;
    std::vector<ValueType> col_types_;
    std::vector<std::vector<Value>> rows_;
    bool dirty_ = false;
};