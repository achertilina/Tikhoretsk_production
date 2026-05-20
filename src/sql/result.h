#pragma once

#include "../core/value.h"
#include <vector>
#include <string>

struct QueryResult {
    enum Type { SELECT, AFFECTED, ERROR };

    Type type;
    std::vector<std::string> column_names;
    std::vector<std::vector<Value>> rows;
    int affected_rows;
    std::string error_message;

    static QueryResult makeSelect(const std::vector<std::string>& cols, const std::vector<std::vector<Value>>& r) {
        QueryResult res;
        res.type = SELECT;
        res.column_names = cols;
        res.rows = r;
        return res;
    }
    static QueryResult makeAffected(int n) {
        QueryResult res;
        res.type = AFFECTED;
        res.affected_rows = n;
        return res;
    }
    static QueryResult makeError(const std::string& msg) {
        QueryResult res;
        res.type = ERROR;
        res.error_message = msg;
        return res;
    }
};