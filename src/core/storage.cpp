#include "storage.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

static json valueToJson(const Value& v) {
    if (v.isNull()) return nullptr;
    switch (v.type()) {
        case ValueType::INT: return v.asInt();
        case ValueType::FLOAT: return v.asFloat();
        case ValueType::TEXT: return v.asText();
        case ValueType::BOOL: return v.asBool();
        default: return nullptr;
    }
}

static Value jsonToValue(const json& j, ValueType typeHint) {
    if (j.is_null()) return Value(nullptr);
    if (typeHint == ValueType::INT) return Value(j.get<int>());
    if (typeHint == ValueType::FLOAT) return Value(j.get<float>());
    if (typeHint == ValueType::BOOL) return Value(j.get<bool>());
    // TEXT or fallback
    return Value(j.get<std::string>());
}

void Storage::save(const Table& table, const std::string& filepath) {
    json j;
    j["table_name"] = table.name();
    // schema
    auto& cols = j["columns"];
    for (size_t i = 0; i < table.columnNames().size(); ++i) {
        json col;
        col["name"] = table.columnNames()[i];
        switch (table.columnTypes()[i]) {
            case ValueType::INT: col["type"] = "INT"; break;
            case ValueType::FLOAT: col["type"] = "FLOAT"; break;
            case ValueType::TEXT: col["type"] = "TEXT"; break;
            case ValueType::BOOL: col["type"] = "BOOL"; break;
            default: col["type"] = "NULL"; break;
        }
        cols.push_back(col);
    }
    // rows
    auto& rows = j["rows"];
    for (const auto& row : table.rows()) {
        json row_json;
        for (size_t i = 0; i < row.size(); ++i) {
            row_json.push_back(valueToJson(row[i]));
        }
        rows.push_back(row_json);
    }
    std::ofstream ofs(filepath);
    if (!ofs) throw std::runtime_error("Cannot write to " + filepath);
    ofs << j.dump(4);
}

Table Storage::load(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs) throw std::runtime_error("Cannot open " + filepath);
    json j;
    ifs >> j;
    std::string tbl_name = j["table_name"];
    Table table(tbl_name);
    // restore schema
    std::vector<std::string> col_names;
    std::vector<ValueType> col_types;
    for (const auto& col : j["columns"]) {
        col_names.push_back(col["name"]);
        std::string typ = col["type"];
        if (typ == "INT") col_types.push_back(ValueType::INT);
        else if (typ == "FLOAT") col_types.push_back(ValueType::FLOAT);
        else if (typ == "TEXT") col_types.push_back(ValueType::TEXT);
        else if (typ == "BOOL") col_types.push_back(ValueType::BOOL);
        else col_types.push_back(ValueType::NULL_TYPE);
    }
    table.setSchema(col_names, col_types);
    // restore rows
    for (const auto& row_json : j["rows"]) {
        std::vector<Value> row;
        for (size_t i = 0; i < row_json.size(); ++i) {
            row.push_back(jsonToValue(row_json[i], col_types[i]));
        }
        table.insertRow(row);
    }
    table.setDirty(false);
    return table;
}