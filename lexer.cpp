#include "lexer.h"
#include <cctype>

/**
 * Se convierte el codigo a tokens.
 *
 * Va por la string caracter por caracter, y lo transforma en una lista de diversos tokens como :
 * - Numeros
 * - Identificadores
 * - Simbolos
 *
 * El valor que regresa esta funcion son tokens procesados
 */

std::vector<Token> tokenize(const std::string& code) { /* Esta funcion convierte el codigo a Tokens*/
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < code.size()) {
        char c = code[i]; /* El caracter actual */

        if (isspace(c)) { /* Se ignoran los espacios */
            i++;
            continue;
        }

        if (isdigit(c)) {
            std::string num;
            while (i < code.size() && isdigit(code[i])) { /* Lee el numero */
                num += code[i++];
            }
            tokens.push_back({TOK_NUMBER, num}); /* Indica que es un numero y cual */
            continue;
        }

        if (isalpha(c) || c == '_') { /* Los Identificadores */
            std::string id;
            while (i < code.size() && (isalnum(code[i]) || code[i] == '_')) {
                id += code[i++];
            }

            tokens.push_back({TOK_IDENTIFIER, id}); 

            continue;
        }
        
        /* Simbolos */
        tokens.push_back({TOK_RBRACE, std::string(1, c)});

        i++;
    }

    tokens.push_back({TOK_EOF, ""}); /* Indica el fin del fichero */
    return tokens;
}

