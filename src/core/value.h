#pragma once

#include <variant>
#include <string>
#include <stdexcept>
#include <iostream>

enum class ValueType {
    INT,
    FLOAT,
    TEXT,
    BOOL,
    NULL_TYPE
};

class Value {
public:
    Value();
    explicit Value(int v);
    explicit Value(float v);
    explicit Value(const std::string& v);
    explicit Value(bool v);
    explicit Value(std::nullptr_t);   // NULL

    ValueType type() const { return type_; }

    int asInt() const;
    float asFloat() const;
    std::string asText() const;
    bool asBool() const;
    bool isNull() const;

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>=(const Value& other) const;

    std::string toString() const;
    static Value fromString(const std::string& str, ValueType hint = ValueType::TEXT);

private:
    std::variant<int, float, std::string, bool, std::monostate> data_;
    ValueType type_;
};