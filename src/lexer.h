#pragma once
#include <string>
#include <vector>

enum TokenType {
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_RBRACE,
    TOK_STRING,
    TOK_EOF
};

struct Token {
    TokenType type;
    std::string value;
};

std::vector<Token> tokenize(const std::string& code);