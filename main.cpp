#include <iostream>
#include "lexer.h"

int main() {
    std::string code = "int a = 5 + 3;";
    auto tokens = tokenize(code);
    int i = 0;

    auto peek = [&]() -> const Token& {
        return tokens[i];
    };
    auto consume = [&]() -> const Token& {
        return tokens[i++];
    };
    auto expect = [&](TokenType type) {
        if (peek().type != type) {
            std::cerr << "Error inesperado\n";
            exit(1);
        }
        return consume();
    };

    while (i < tokens.size()) {
        if (peek().type == TOK_IDENTIFIER) {
            consume();
            std::cout << "identifier" << std::endl;
        }
        else if (peek().type == TOK_NUMBER) {
            consume();
            std::cout << "number" << std::endl;
        }
        else if (peek().type == TOK_RBRACE) {
            consume();
            std::cout << "symbol" << std::endl;
        }
        else if (peek().type == TOK_EOF) break;
    }
}