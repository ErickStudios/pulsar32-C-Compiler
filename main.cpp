#include <iostream>
#include "lexer.h"

/**
 * Este programa toma una string de codigo fuente, 
 * la convierte a Tokens usando el Lexer, 
 * y luego imprime que tipo de token es
 */

int main() {
    std::string code = "int a = 5 + 3;"; /* Codigo a ser procesado */
    auto tokens = tokenize(code); /* Esto nos convierte el texto a tokens */
    int i = 0;

    /* HERRAMIENTAS */
    auto peek = [&]() -> const Token& { /* Mirar el token actual, sin consumirlo */
        return tokens[i];
    };
    auto consume = [&]() -> const Token& { /* Usar el token, y avanzar al siguiente */
        return tokens[i++];
    };
    auto expect = [&](TokenType type) { /* Espera un tipo de token, si no, da error */
        if (peek().type != type) {
            std::cerr << "Error inesperado\n";
            exit(1);
        }
        return consume();
    };

    while (i < tokens.size()) {
        if (peek().type == TOK_IDENTIFIER) { /* Si es un identificador, se imprime que es un identificador y avanza al siguiente token! */
            consume();
            std::cout << "identifier" << std::endl;
        }
        else if (peek().type == TOK_NUMBER) { /* Si el token es un numero, se imprime que es un numero y se avanza al siguiente */
            consume();
            std::cout << "number" << std::endl;
        }
        else if (peek().type == TOK_RBRACE) { /* Si el token es un simbolo, se imprime que es un simbolo y se avanza al siguiente */
            consume();
            std::cout << "symbol" << std::endl;
        }
        else if (peek().type == TOK_EOF) break; /* Si es el final de programa, ahi termina todo */
    }
}
