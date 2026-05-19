#include "parser.h"
#include <cctype>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>

enum TokenType {
    TOK_KEYWORD,
    TOK_IDENT,
    TOK_STRING,
    TOK_NUMBER,
    TOK_OPERATOR,
    TOK_PUNCTUATION,
    TOK_EOF
};

struct Token {
    TokenType type;
    std::string value;
    int line, col;
};

class Lexer {
    std::string src;
    size_t pos;
    int line, col;
public:
    Lexer(const std::string& s) : src(s), pos(0), line(1), col(1) {}

    Token next() {
        skipWhitespace();
        if (pos >= src.size()) return {TOK_EOF, "", line, col};
        char c = src[pos];
        // Numbers
        if (std::isdigit(c) || (c == '-' && pos+1 < src.size() && std::isdigit(src[pos+1]))) {
            std::string num;
            if (c == '-') { num += c; pos++; col++; }
            while (pos < src.size() && std::isdigit(src[pos])) {
                num += src[pos]; pos++; col++;
            }
            if (pos < src.size() && src[pos] == '.') {
                num += '.'; pos++; col++;
                while (pos < src.size() && std::isdigit(src[pos])) {
                    num += src[pos]; pos++; col++;
                }
            }
            return {TOK_NUMBER, num, line, col};
        }
        // String literals
        if (c == '\'') {
            pos++; col++;
            std::string str;
            while (pos < src.size() && src[pos] != '\'') {
                str += src[pos]; pos++; col++;
            }
            if (pos >= src.size()) throw std::runtime_error("Unterminated string");
            pos++; col++;
            return {TOK_STRING, str, line, col};
        }
        // Operators
        if (c == '=') { pos++; col++; return {TOK_OPERATOR, "=", line, col}; }
        if (c == '!') {
            if (pos+1 < src.size() && src[pos+1] == '=') {
                pos+=2; col+=2; return {TOK_OPERATOR, "!=", line, col};
            }
            throw std::runtime_error("Unexpected '!'");
        }
        if (c == '<') {
            if (pos+1 < src.size() && src[pos+1] == '=') {
                pos+=2; col+=2; return {TOK_OPERATOR, "<=", line, col};
            }
            pos++; col++; return {TOK_OPERATOR, "<", line, col};
        }
        if (c == '>') {
            if (pos+1 < src.size() && src[pos+1] == '=') {
                pos+=2; col+=2; return {TOK_OPERATOR, ">=", line, col};
            }
            pos++; col++; return {TOK_OPERATOR, ">", line, col};
        }
        // Punctuation
        if (c == '(' || c == ')' || c == ',' || c == ';') {
            std::string punc(1, c);
            pos++; col++;
            return {TOK_PUNCTUATION, punc, line, col};
        }
        // Identifiers and keywords
        if (std::isalpha(c) || c == '_') {
            std::string ident;
            while (pos < src.size() && (std::isalnum(src[pos]) || src[pos] == '_')) {
                ident += src[pos]; pos++; col++;
            }
            std::string upper = ident;
            std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
            static const std::vector<std::string> keywords = {
                "SELECT","FROM","WHERE","INSERT","INTO","VALUES","UPDATE","SET","DELETE",
                "CREATE","DATABASE","TABLE","DROP","AND","OR","NOT","NULL","TRUE","FALSE",
                "INT","FLOAT","BOOL","TEXT","VARCHAR"
            };
            if (std::find(keywords.begin(), keywords.end(), upper) != keywords.end())
                return {TOK_KEYWORD, upper, line, col};
            else
                return {TOK_IDENT, ident, line, col};
        }
        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

private:
    void skipWhitespace() {
        while (pos < src.size() && std::isspace(src[pos])) {
            if (src[pos] == '\n') { line++; col=1; } else col++;
            pos++;
        }
    }
};

class ParserImpl {
    Lexer lex;
    Token curr;
public:
    ParserImpl(const std::string& sql) : lex(sql) { advance(); }

    std::unique_ptr<Statement> parseStatement() {
        if (curr.type == TOK_KEYWORD) {
            std::string kw = curr.value;
            if (kw == "SELECT") return parseSelect();
            if (kw == "INSERT") return parseInsert();
            if (kw == "UPDATE") return parseUpdate();
            if (kw == "DELETE") return parseDelete();
            if (kw == "CREATE") return parseCreate();
            if (kw == "DROP") return parseDrop();
        }
        throw std::runtime_error("Unknown statement");
    }

private:
    void advance() { curr = lex.next(); }

    void expect(TokenType type, const std::string& value = "") {
        if (curr.type != type)
            throw std::runtime_error("Expected token type " + std::to_string(type) + " but got " + std::to_string(curr.type) + " ('" + curr.value + "')");
        if (!value.empty() && curr.value != value)
            throw std::runtime_error("Expected value '" + value + "' but got '" + curr.value + "'");
        advance();
    }

    // --- SELECT ---
    std::unique_ptr<SelectQuery> parseSelect() {
        auto q = std::make_unique<SelectQuery>();
        advance(); // SELECT
        // projection
        if (curr.type == TOK_KEYWORD && curr.value == "*") {
            advance();
        } else {
            do {
                if (curr.type != TOK_IDENT) throw std::runtime_error("Expected column name");
                q->columns.push_back(curr.value);
                advance();
                if (curr.type == TOK_PUNCTUATION && curr.value == ",") advance();
                else break;
            } while (true);
        }
        expect(TOK_KEYWORD, "FROM");
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        if (curr.type == TOK_KEYWORD && curr.value == "WHERE") {
            advance();
            q->where = parseCondition();
        }
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- INSERT ---
    std::unique_ptr<InsertQuery> parseInsert() {
        auto q = std::make_unique<InsertQuery>();
        advance(); // INSERT
        expect(TOK_KEYWORD, "INTO");
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        // optional column list
        if (curr.type == TOK_PUNCTUATION && curr.value == "(") {
            advance();
            do {
                if (curr.type != TOK_IDENT) throw std::runtime_error("Expected column name");
                q->columns.push_back(curr.value);
                advance();
                if (curr.type == TOK_PUNCTUATION && curr.value == ",") advance();
                else break;
            } while (true);
            expect(TOK_PUNCTUATION, ")");
        }
        expect(TOK_KEYWORD, "VALUES");
        do {
            expect(TOK_PUNCTUATION, "(");
            std::vector<Value> row;
            do {
                row.push_back(parseValue());
                if (curr.type == TOK_PUNCTUATION && curr.value == ",") advance();
                else break;
            } while (true);
            expect(TOK_PUNCTUATION, ")");
            q->rows.push_back(row);
            if (curr.type == TOK_PUNCTUATION && curr.value == ",") advance();
            else break;
        } while (true);
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- UPDATE ---
    std::unique_ptr<UpdateQuery> parseUpdate() {
        auto q = std::make_unique<UpdateQuery>();
        advance(); // UPDATE
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        expect(TOK_KEYWORD, "SET");
        do {
            if (curr.type != TOK_IDENT) throw std::runtime_error("Expected column name");
            std::string col = curr.value;
            advance();
            expect(TOK_OPERATOR, "=");
            Value val = parseValue();
            q->assignments[col] = val;
            if (curr.type == TOK_PUNCTUATION && curr.value == ",") advance();
            else break;
        } while (true);
        if (curr.type == TOK_KEYWORD && curr.value == "WHERE") {
            advance();
            q->where = parseCondition();
        }
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- DELETE ---
    std::unique_ptr<DeleteQuery> parseDelete() {
        auto q = std::make_unique<DeleteQuery>();
        advance(); // DELETE
        expect(TOK_KEYWORD, "FROM");
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        if (curr.type == TOK_KEYWORD && curr.value == "WHERE") {
            advance();
            q->where = parseCondition();
        }
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- CREATE DATABASE / TABLE ---
    std::unique_ptr<Statement> parseCreate() {
        advance(); // consume CREATE
        if (curr.type != TOK_KEYWORD) throw std::runtime_error("Expected DATABASE or TABLE");
        std::string kw = curr.value;
        if (kw == "DATABASE") return parseCreateDatabase();
        if (kw == "TABLE") return parseCreateTable();
        throw std::runtime_error("Expected DATABASE or TABLE after CREATE");
    }

    std::unique_ptr<CreateDatabaseQuery> parseCreateDatabase() {
        auto q = std::make_unique<CreateDatabaseQuery>();
        advance(); // consume DATABASE
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected database name");
        q->database_name = curr.value;
        advance(); // consume name
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    std::unique_ptr<CreateTableQuery> parseCreateTable() {
        auto q = std::make_unique<CreateTableQuery>();
        advance(); // consume TABLE
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        expect(TOK_PUNCTUATION, "(");
        do {
            if (curr.type != TOK_IDENT) throw std::runtime_error("Expected column name");
            std::string col_name = curr.value;
            advance();
            if (curr.type != TOK_KEYWORD) throw std::runtime_error("Expected data type");
            std::string type_str = curr.value;
            ValueType col_type;
            if (type_str == "INT") col_type = ValueType::INT;
            else if (type_str == "FLOAT") col_type = ValueType::FLOAT;
            else if (type_str == "BOOL") col_type = ValueType::BOOL;
            else if (type_str == "TEXT") col_type = ValueType::TEXT;
            else if (type_str == "VARCHAR") {
                col_type = ValueType::TEXT;
                advance();
                if (curr.type == TOK_PUNCTUATION && curr.value == "(") {
                    advance();
                    if (curr.type == TOK_NUMBER) advance();
                    expect(TOK_PUNCTUATION, ")");
                } else {
                    throw std::runtime_error("Expected (size) after VARCHAR");
                }
            } else {
                throw std::runtime_error("Unknown data type");
            }
            advance();
            q->column_names.push_back(col_name);
            q->column_types.push_back(col_type);
            if (curr.type == TOK_PUNCTUATION && curr.value == ",") {
                advance();
                continue;
            } else break;
        } while (true);
        expect(TOK_PUNCTUATION, ")");
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- DROP DATABASE / TABLE ---
    std::unique_ptr<Statement> parseDrop() {
        advance(); // consume DROP
        if (curr.type != TOK_KEYWORD) throw std::runtime_error("Expected DATABASE or TABLE");
        std::string kw = curr.value;
        if (kw == "DATABASE") return parseDropDatabase();
        if (kw == "TABLE") return parseDropTable();
        throw std::runtime_error("Expected DATABASE or TABLE after DROP");
    }

    std::unique_ptr<DropDatabaseQuery> parseDropDatabase() {
        auto q = std::make_unique<DropDatabaseQuery>();
        advance(); // consume DATABASE
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected database name");
        q->database_name = curr.value;
        advance();
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    std::unique_ptr<DropTableQuery> parseDropTable() {
        auto q = std::make_unique<DropTableQuery>();
        advance(); // consume TABLE
        if (curr.type != TOK_IDENT) throw std::runtime_error("Expected table name");
        q->table_name = curr.value;
        advance();
        expect(TOK_PUNCTUATION, ";");
        return q;
    }

    // --- Condition parsing ---
    std::unique_ptr<Condition> parseCondition() {
        return parseOr();
    }

    std::unique_ptr<Condition> parseOr() {
        auto left = parseAnd();
        while (curr.type == TOK_KEYWORD && curr.value == "OR") {
            advance();
            auto right = parseAnd();
            auto logic = std::make_unique<LogicalCondition>();
            logic->logic = ConditionType::LOGICAL_OR;
            logic->operands.push_back(std::move(left));
            logic->operands.push_back(std::move(right));
            left = std::move(logic);
        }
        return left;
    }

    std::unique_ptr<Condition> parseAnd() {
        auto left = parseNot();
        while (curr.type == TOK_KEYWORD && curr.value == "AND") {
            advance();
            auto right = parseNot();
            auto logic = std::make_unique<LogicalCondition>();
            logic->logic = ConditionType::LOGICAL_AND;
            logic->operands.push_back(std::move(left));
            logic->operands.push_back(std::move(right));
            left = std::move(logic);
        }
        return left;
    }

    std::unique_ptr<Condition> parseNot() {
        if (curr.type == TOK_KEYWORD && curr.value == "NOT") {
            advance();
            auto cond = parsePrimary();
            auto notCond = std::make_unique<NotCondition>();
            notCond->operand = std::move(cond);
            return notCond;
        }
        return parsePrimary();
    }

    std::unique_ptr<Condition> parsePrimary() {
        if (curr.type == TOK_PUNCTUATION && curr.value == "(") {
            advance();
            auto cond = parseCondition();
            expect(TOK_PUNCTUATION, ")");
            return cond;
        }
        if (curr.type == TOK_IDENT) {
            std::string col = curr.value;
            advance();
            if (curr.type != TOK_OPERATOR) throw std::runtime_error("Expected operator");
            std::string op = curr.value;
            advance();
            Value val = parseValue();
            auto cmp = std::make_unique<ComparisonCondition>();
            cmp->column = col;
            cmp->op = op;
            cmp->value = val;
            return cmp;
        }
        throw std::runtime_error("Expected condition");
    }

    Value parseValue() {
        if (curr.type == TOK_KEYWORD) {
            if (curr.value == "NULL") { advance(); return Value(nullptr); }
            if (curr.value == "TRUE") { advance(); return Value(true); }
            if (curr.value == "FALSE") { advance(); return Value(false); }
            throw std::runtime_error("Unexpected keyword in value");
        }
        if (curr.type == TOK_STRING) {
            std::string s = curr.value;
            advance();
            return Value(s);
        }
        if (curr.type == TOK_NUMBER) {
            std::string num = curr.value;
            advance();
            if (num.find('.') != std::string::npos)
                return Value(std::stof(num));
            else
                return Value(std::stoi(num));
        }
        throw std::runtime_error("Expected value");
    }
};

std::unique_ptr<Statement> Parser::parse(const std::string& sql) {
    ParserImpl parser(sql);
    return parser.parseStatement();
}