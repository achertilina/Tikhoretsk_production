#include "value.h"
#include <cctype>

Value::Value() : type_(ValueType::NULL_TYPE), data_(std::monostate{}) {}

Value::Value(int v) : type_(ValueType::INT), data_(v) {}
Value::Value(float v) : type_(ValueType::FLOAT), data_(v) {}
Value::Value(const std::string& v) : type_(ValueType::TEXT), data_(v) {}
Value::Value(bool v) : type_(ValueType::BOOL), data_(v) {}
Value::Value(std::nullptr_t) : type_(ValueType::NULL_TYPE), data_(std::monostate{}) {}

int Value::asInt() const {
    if (type_ != ValueType::INT) throw std::runtime_error("Not an INT");
    return std::get<int>(data_);
}

float Value::asFloat() const {
    if (type_ != ValueType::FLOAT) throw std::runtime_error("Not a FLOAT");
    return std::get<float>(data_);
}

std::string Value::asText() const {
    if (type_ != ValueType::TEXT) throw std::runtime_error("Not a TEXT");
    return std::get<std::string>(data_);
}

bool Value::asBool() const {
    if (type_ != ValueType::BOOL) throw std::runtime_error("Not a BOOL");
    return std::get<bool>(data_);
}

bool Value::isNull() const {
    return type_ == ValueType::NULL_TYPE;
}

bool Value::operator==(const Value& other) const {
    if (type_ != other.type_) return false;
    switch (type_) {
        case ValueType::INT: return std::get<int>(data_) == std::get<int>(other.data_);
        case ValueType::FLOAT: return std::get<float>(data_) == std::get<float>(other.data_);
        case ValueType::TEXT: return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        case ValueType::BOOL: return std::get<bool>(data_) == std::get<bool>(other.data_);
        case ValueType::NULL_TYPE: return true;
    }
    return false;
}

bool Value::operator!=(const Value& other) const { return !(*this == other); }

bool Value::operator<(const Value& other) const {
    if (type_ != other.type_) throw std::runtime_error("Type mismatch in comparison");
    switch (type_) {
        case ValueType::INT: return std::get<int>(data_) < std::get<int>(other.data_);
        case ValueType::FLOAT: return std::get<float>(data_) < std::get<float>(other.data_);
        case ValueType::TEXT: return std::get<std::string>(data_) < std::get<std::string>(other.data_);
        case ValueType::BOOL: return std::get<bool>(data_) < std::get<bool>(other.data_);
        default: return false;
    }
}

bool Value::operator>(const Value& other) const { return other < *this; }
bool Value::operator<=(const Value& other) const { return !(*this > other); }
bool Value::operator>=(const Value& other) const { return !(*this < other); }

std::string Value::toString() const {
    if (isNull()) return "NULL";
    switch (type_) {
        case ValueType::INT: return std::to_string(std::get<int>(data_));
        case ValueType::FLOAT: return std::to_string(std::get<float>(data_));
        case ValueType::TEXT: return std::get<std::string>(data_);
        case ValueType::BOOL: return std::get<bool>(data_) ? "TRUE" : "FALSE";
        default: return "";
    }
}

Value Value::fromString(const std::string& str, ValueType hint) {
    if (str == "NULL") return Value(nullptr);
    if (hint == ValueType::INT) {
        try { return Value(std::stoi(str)); } catch (...) {}
    } else if (hint == ValueType::FLOAT) {
        try { return Value(std::stof(str)); } catch (...) {}
    } else if (hint == ValueType::BOOL) {
        if (str == "TRUE" || str == "true" || str == "1") return Value(true);
        if (str == "FALSE" || str == "false" || str == "0") return Value(false);
    }
    // fallback to text
    return Value(str);
}