#pragma once
#include <string>
#include <vector>

enum TokenType { /* Esta enumeracion define los tokens de el compilador */
    TOK_IDENTIFIER, /* Nombres, por ejemplo, los de las variables */
    TOK_NUMBER,
    TOK_RBRACE,
    TOK_EOF /* End Of File */
};

struct Token { /* Define que es un Token */
    TokenType type; /* ¿Cual token es?(Vease TokenType) */
    std::string value; /* Valor : Esto nos dice que texto tenia */
};

/*==FUNCIONES==*/

std::vector<Token> tokenize(const std::string& code);
