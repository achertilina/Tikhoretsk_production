#pragma once

#include "ast.h"
#include <memory>
#include <string>

class Parser {
public:
    std::unique_ptr<Statement> parse(const std::string& sql);
};