#include "lexer.h"
#include "ast.h"
#include <cctype>

std::vector<Token> tokenize(const std::string& code) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < code.size()) {
        char c = code[i];

        if (isspace(c)) {
            i++;
            continue;
        }

        if (isdigit(c)) {
            std::string num;
            while (i < code.size() && isdigit(code[i])) {
                num += code[i++];
            }
            tokens.push_back({TOK_NUMBER, num});
            continue;
        }

        if (c == '"') {
            i++;
            std::string str_val = "";
            
            while (i < code.size() && code[i] != '"') {
                if (code[i] == '\\') {
                    i++;
                    if (i >= code.size()) break;
                    
                    char escape = code[i];
                    if (escape == 'n')  str_val += '\n';
                    else if (escape == 't') str_val += '\t';
                    else if (escape == 'r') str_val += '\r'; 
                    else if (escape == '\\') str_val += '\\';
                    else if (escape == '"')  str_val += '"'; 
                    else {
                        str_val += escape; 
                    }
                } else {
                    str_val += code[i];
                }
                i++;
            }
            
            if (i < code.size() && code[i] == '"') {
                i++;
            }

            tokens.push_back({TOK_STRING, str_val}); 
            continue;
        }

        if (isalpha(c) || c == '_') {
            std::string id;
            while (i < code.size() && (isalnum(code[i]) || code[i] == '_')) {
                id += code[i++];
            }

            tokens.push_back({TOK_IDENTIFIER, id});

            continue;
        }

        tokens.push_back({TOK_RBRACE, std::string(1, c)});

        i++;
    }

    tokens.push_back({TOK_EOF, ""});
    return tokens;
}