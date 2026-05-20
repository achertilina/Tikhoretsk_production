#include "executor.h"
#include <unordered_map>
#include <stdexcept>

Executor::Executor(Catalog* catalog) : catalog_(catalog) {}

void Executor::setCurrentDatabase(const std::string& db_name) {
    current_db_ = db_name;
}

QueryResult Executor::execute(const Statement& stmt) {
    switch (stmt.stmtType()) {
        case StatementType::SELECT:
            return executeSelect(static_cast<const SelectQuery&>(stmt));
        case StatementType::INSERT:
            return executeInsert(static_cast<const InsertQuery&>(stmt));
        case StatementType::UPDATE:
            return executeUpdate(static_cast<const UpdateQuery&>(stmt));
        case StatementType::DELETE:
            return executeDelete(static_cast<const DeleteQuery&>(stmt));
        case StatementType::CREATE_DATABASE:
            return executeCreateDatabase(static_cast<const CreateDatabaseQuery&>(stmt));
        case StatementType::DROP_DATABASE:
            return executeDropDatabase(static_cast<const DropDatabaseQuery&>(stmt));
        case StatementType::CREATE_TABLE:
            return executeCreateTable(static_cast<const CreateTableQuery&>(stmt));
        case StatementType::DROP_TABLE:
            return executeDropTable(static_cast<const DropTableQuery&>(stmt));
        default:
            throw std::runtime_error("Unknown statement type");
    }
}

bool Executor::evaluateCondition(const Condition* cond, const Table& table, size_t row_idx) {
    if (!cond) return true;

    switch (cond->type()) {
        case ConditionType::COMPARISON: {
            auto cmp = static_cast<const ComparisonCondition*>(cond);
            const auto& row = table.rows()[row_idx];
            size_t col_idx = 0;
            const auto& col_names = table.columnNames();
            for (; col_idx < col_names.size(); ++col_idx) {
                if (col_names[col_idx] == cmp->column) break;
            }
            if (col_idx == col_names.size())
                throw std::runtime_error("Column not found: " + cmp->column);

            const Value& col_val = row[col_idx];
            const Value& lit_val = cmp->value;

            if (cmp->op == "=")  return col_val == lit_val;
            if (cmp->op == "!=") return col_val != lit_val;
            if (cmp->op == "<")  return col_val < lit_val;
            if (cmp->op == ">")  return col_val > lit_val;
            if (cmp->op == "<=") return col_val <= lit_val;
            if (cmp->op == ">=") return col_val >= lit_val;
            throw std::runtime_error("Unknown comparison operator: " + cmp->op);
        }
        case ConditionType::LOGICAL_AND: {
            auto logic = static_cast<const LogicalCondition*>(cond);
            for (const auto& op : logic->operands)
                if (!evaluateCondition(op.get(), table, row_idx))
                    return false;
            return true;
        }
        case ConditionType::LOGICAL_OR: {
            auto logic = static_cast<const LogicalCondition*>(cond);
            for (const auto& op : logic->operands)
                if (evaluateCondition(op.get(), table, row_idx))
                    return true;
            return false;
        }
        case ConditionType::NOT: {
            auto notc = static_cast<const NotCondition*>(cond);
            return !evaluateCondition(notc->operand.get(), table, row_idx);
        }
        default:
            return true;
    }
}

// ==================== Проверка наличия выбранной БД ====================
static void ensureDatabaseSelected(const std::string& current_db) {
    if (current_db.empty()) {
        throw std::runtime_error("No database selected. Please use USE <database> or CREATE DATABASE first (it auto-selects).");
    }
}

// ==================== CREATE DATABASE (автоматически переключает) ====================
QueryResult Executor::executeCreateDatabase(const CreateDatabaseQuery& q) {
    catalog_->createDatabase(q.database_name);
    current_db_ = q.database_name;   // автоматически переключаемся на созданную БД
    return QueryResult::makeAffected(0);
}

// ==================== DROP DATABASE ====================
QueryResult Executor::executeDropDatabase(const DropDatabaseQuery& q) {
    catalog_->dropDatabase(q.database_name);
    if (current_db_ == q.database_name) {
        current_db_.clear();   // если удалили текущую БД – сбрасываем выбор
    }
    return QueryResult::makeAffected(0);
}

// ==================== CREATE TABLE ====================
QueryResult Executor::executeCreateTable(const CreateTableQuery& q) {
    ensureDatabaseSelected(current_db_);
    catalog_->createTable(current_db_, q.table_name, q.column_names, q.column_types);
    return QueryResult::makeAffected(0);
}

// ==================== DROP TABLE ====================
QueryResult Executor::executeDropTable(const DropTableQuery& q) {
    ensureDatabaseSelected(current_db_);
    catalog_->dropTable(current_db_, q.table_name);
    return QueryResult::makeAffected(0);
}

// ==================== SELECT ====================
QueryResult Executor::executeSelect(const SelectQuery& q) {
    ensureDatabaseSelected(current_db_);
    Table& table = catalog_->getTable(current_db_, q.table_name);

    std::vector<size_t> row_indices;
    for (size_t i = 0; i < table.rowCount(); ++i) {
        if (evaluateCondition(q.where.get(), table, i))
            row_indices.push_back(i);
    }

    std::vector<size_t> col_indices;
    if (q.columns.empty()) {
        for (size_t i = 0; i < table.columnNames().size(); ++i)
            col_indices.push_back(i);
    } else {
        for (const auto& col_name : q.columns) {
            bool found = false;
            for (size_t i = 0; i < table.columnNames().size(); ++i) {
                if (table.columnNames()[i] == col_name) {
                    col_indices.push_back(i);
                    found = true;
                    break;
                }
            }
            if (!found)
                throw std::runtime_error("Column not found: " + col_name);
        }
    }

    std::vector<std::string> result_col_names;
    for (size_t idx : col_indices)
        result_col_names.push_back(table.columnNames()[idx]);

    std::vector<std::vector<Value>> result_rows;
    for (size_t row_idx : row_indices) {
        std::vector<Value> row;
        for (size_t col_idx : col_indices)
            row.push_back(table.rows()[row_idx][col_idx]);
        result_rows.push_back(std::move(row));
    }

    return QueryResult::makeSelect(result_col_names, result_rows);
}

// ==================== INSERT ====================
QueryResult Executor::executeInsert(const InsertQuery& q) {
    ensureDatabaseSelected(current_db_);
    Table& table = catalog_->getTable(current_db_, q.table_name);

    std::vector<size_t> target_col_indices;
    if (q.columns.empty()) {
        for (size_t i = 0; i < table.columnNames().size(); ++i)
            target_col_indices.push_back(i);
    } else {
        for (const auto& col_name : q.columns) {
            bool found = false;
            for (size_t i = 0; i < table.columnNames().size(); ++i) {
                if (table.columnNames()[i] == col_name) {
                    target_col_indices.push_back(i);
                    found = true;
                    break;
                }
            }
            if (!found)
                throw std::runtime_error("Column not found: " + col_name);
        }
    }

    int affected = 0;
    for (const auto& row_vals : q.rows) {
        if (row_vals.size() != target_col_indices.size())
            throw std::runtime_error("Number of values does not match number of columns");

        std::vector<Value> full_row(table.columnNames().size(), Value(nullptr));
        for (size_t j = 0; j < target_col_indices.size(); ++j) {
            full_row[target_col_indices[j]] = row_vals[j];
        }
        table.insertRow(full_row);
        ++affected;
    }

    return QueryResult::makeAffected(affected);
}

// ==================== UPDATE ====================
QueryResult Executor::executeUpdate(const UpdateQuery& q) {
    ensureDatabaseSelected(current_db_);
    Table& table = catalog_->getTable(current_db_, q.table_name);

    std::vector<size_t> row_indices;
    for (size_t i = 0; i < table.rowCount(); ++i) {
        if (evaluateCondition(q.where.get(), table, i))
            row_indices.push_back(i);
    }

    std::unordered_map<size_t, Value> updates_by_index;
    for (const auto& [col_name, new_val] : q.assignments) {
        size_t col_idx = 0;
        for (; col_idx < table.columnNames().size(); ++col_idx)
            if (table.columnNames()[col_idx] == col_name) break;
        if (col_idx == table.columnNames().size())
            throw std::runtime_error("Column not found: " + col_name);
        updates_by_index[col_idx] = new_val;
    }

    for (size_t row_idx : row_indices) {
        for (const auto& [col_idx, new_val] : updates_by_index) {
            table.updateRow(row_idx, {{table.columnNames()[col_idx], new_val}});
        }
    }

    return QueryResult::makeAffected(row_indices.size());
}

// ==================== DELETE ====================
QueryResult Executor::executeDelete(const DeleteQuery& q) {
    ensureDatabaseSelected(current_db_);
    Table& table = catalog_->getTable(current_db_, q.table_name);

    std::vector<size_t> row_indices;
    for (size_t i = 0; i < table.rowCount(); ++i) {
        if (evaluateCondition(q.where.get(), table, i))
            row_indices.push_back(i);
    }

    for (auto it = row_indices.rbegin(); it != row_indices.rend(); ++it)
        table.deleteRow(*it);

    return QueryResult::makeAffected(row_indices.size());
}