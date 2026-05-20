#pragma once

#include "../core/value.h"
#include <memory>
#include <vector>
#include <string>

#include <unordered_map>

// Forward declarations
struct Condition;
struct Statement;

// Condition types
enum class ConditionType { COMPARISON, LOGICAL_AND, LOGICAL_OR, NOT };

struct Condition {
    virtual ~Condition() = default;
    virtual ConditionType type() const = 0;
};

struct ComparisonCondition : Condition {
    std::string column;
    std::string op;   // "=", "!=", "<", ">", "<=", ">="
    Value value;
    ConditionType type() const override { return ConditionType::COMPARISON; }
};

struct LogicalCondition : Condition {
    ConditionType logic; // AND or OR
    std::vector<std::unique_ptr<Condition>> operands;
    ConditionType type() const override { return logic; }
};

struct NotCondition : Condition {
    std::unique_ptr<Condition> operand;
    ConditionType type() const override { return ConditionType::NOT; }
};

// Statements
enum class StatementType {
    SELECT, INSERT, UPDATE, DELETE,
    CREATE_DATABASE, DROP_DATABASE,
    CREATE_TABLE, DROP_TABLE
};

struct Statement {
    virtual ~Statement() = default;
    virtual StatementType stmtType() const = 0;
};

struct SelectQuery : Statement {
    std::string table_name;
    std::vector<std::string> columns;  // empty means "*"
    std::unique_ptr<Condition> where;
    StatementType stmtType() const override { return StatementType::SELECT; }
};

struct InsertQuery : Statement {
    std::string table_name;
    std::vector<std::string> columns;  // if empty, then all columns in schema order
    std::vector<std::vector<Value>> rows;
    StatementType stmtType() const override { return StatementType::INSERT; }
};

struct UpdateQuery : Statement {
    std::string table_name;
    std::unordered_map<std::string, Value> assignments;
    std::unique_ptr<Condition> where;
    StatementType stmtType() const override { return StatementType::UPDATE; }
};

struct DeleteQuery : Statement {
    std::string table_name;
    std::unique_ptr<Condition> where;
    StatementType stmtType() const override { return StatementType::DELETE; }
};

struct CreateDatabaseQuery : Statement {
    std::string database_name;
    StatementType stmtType() const override { return StatementType::CREATE_DATABASE; }
};

struct DropDatabaseQuery : Statement {
    std::string database_name;
    StatementType stmtType() const override { return StatementType::DROP_DATABASE; }
};

struct CreateTableQuery : Statement {
    std::string table_name;
    std::vector<std::string> column_names;
    std::vector<ValueType> column_types;
    StatementType stmtType() const override { return StatementType::CREATE_TABLE; }
};

struct DropTableQuery : Statement {
    std::string table_name;
    StatementType stmtType() const override { return StatementType::DROP_TABLE; }
};