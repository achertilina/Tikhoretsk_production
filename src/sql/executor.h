#pragma once

#include "ast.h"
#include "../core/catalog.h"
#include "result.h"   // internal QueryResult

class Executor {
public:
    explicit Executor(Catalog* catalog);

    void setCurrentDatabase(const std::string& db_name);
    std::string getCurrentDatabase() const { return current_db_; }
    QueryResult execute(const Statement& stmt);

private:
    Catalog* catalog_;
    std::string current_db_;

    // Обработчики конкретных запросов
    QueryResult executeSelect(const SelectQuery& q);
    QueryResult executeInsert(const InsertQuery& q);
    QueryResult executeUpdate(const UpdateQuery& q);
    QueryResult executeDelete(const DeleteQuery& q);
    QueryResult executeCreateDatabase(const CreateDatabaseQuery& q);
    QueryResult executeDropDatabase(const DropDatabaseQuery& q);
    QueryResult executeCreateTable(const CreateTableQuery& q);
    QueryResult executeDropTable(const DropTableQuery& q);

    bool evaluateCondition(const Condition* cond, const Table& table, size_t row_idx);
};