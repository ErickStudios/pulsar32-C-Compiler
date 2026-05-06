#include <iostream>
#include "lexer.h"
#include "ast.h"
#include <functional>
#include <fstream>
#include <sstream>

void displayy(BodyCode& program) {
    for (auto stmt : program.body) {
        if (auto var = dynamic_cast<VarDecl*>(stmt)) {
            std::cout << "VarDecl -> "
            << var->name
            << " size: " << var->size
            << "\n";
        }
        else if (auto var = dynamic_cast<FunctionDecl*>(stmt)) {
            std::cout << "FunctionDecl -> "
            << var->nam
            << " size: " << var->size
            << "\n";
            displayy(*var->code);
        }
        else if (auto asg = dynamic_cast<Assignment*>(stmt)) {
            std::cout << "Assignment -> "
                    << asg->name
                    << " = " << asg->value
                    << "\n";
        }
    }
}
std::string parseToSize(VariablesSize size) {
    if (size == VariablesSize::Char) return "Byte";
    if (size == VariablesSize::Short) return "Word";
    if (size == VariablesSize::Int) return "Dword";
    return "Dword";
}
std::string generateCode(BodyCode& program) {
    std::string out;

    for (auto stmt : program.body) {
        if (auto var = dynamic_cast<VarDecl*>(stmt)) {
            out += var->name + ":\n";
            out += "Assume-" + parseToSize(var->size) + " 0\n";
        }
        else if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {
            out += fn->nam + ":\n";

            if (fn->code) {
                out += generateCode(*fn->code);
            }

            out += "HLT\n";
        }
        else if (auto asg = dynamic_cast<Assignment*>(stmt)) {
            out += "Push-Dword " + std::to_string(asg->value) + "\n";
            out += "Pop-Dword " + asg->name + "\n";
        }
    }

    return out;
}
BodyCode parseCode(const std::string& code) {
    auto tokens = tokenize(code);
    int i = 0;

    auto peek = [&]() -> const Token& {
        return tokens[i];
    };
    auto consume = [&]() -> const Token& {
        return tokens[i++];
    };
    auto expect = [&](std::string value) {
        if (peek().value != value) {
            std::cerr << "Error inesperado\n";
            exit(1);
        }
        return consume();
    };
    std::function<StatmentNode*()> parseStatement;
    auto parseType = [&]() -> VariablesSize {
        if (peek().type == TOK_IDENTIFIER && peek().value == "char") {
            consume();
            return VariablesSize::Char;
        }
        if (peek().type == TOK_IDENTIFIER && peek().value == "short") {
            consume();
            return VariablesSize::Short;
        }
        if (peek().type == TOK_IDENTIFIER && peek().value == "int") {
            consume();
            return VariablesSize::Int;
        }

        return VariablesSize::Misingno;
    };
    auto parseAssignment = [&]() -> StatmentNode* {
        if (peek().type == TOK_IDENTIFIER) {
            std::string name = consume().value;

            if (peek().value == "=") {
                consume();

                if (peek().type == TOK_NUMBER) {
                    int value = std::stoi(consume().value);

                    expect(";");

                    auto assign = new Assignment();
                    assign->name = name;
                    assign->value = value;

                    return assign;
                }
            }

            i--;
        }

        return nullptr;
    };
    auto parseVarDecl = [&]() -> StatmentNode* {
        auto type = parseType();

        if (type == VariablesSize::Misingno)
            return nullptr;

        auto name = expect(peek().value).value;

        if (peek().value == "(") {
            auto var = new FunctionDecl();
            var->size = type;
            var->nam = name;
            consume();
            expect(")");
            auto codeNew = new BodyCode();
            expect("{");
            while (peek().value != "}") {
                auto stmt = parseStatement();
                if (stmt) {
                    codeNew->body.push_back(stmt);
                }
            }
            var->code = codeNew;
            return var;
        }
        else { 
            auto var = new VarDecl();

            var->size = type;
            var->name = name;
            expect(";");
            return var;
        }

        return nullptr;
    };

    parseStatement = [&]() -> StatmentNode* {
        auto stmt = parseVarDecl();
        if (stmt) return stmt;

        stmt = parseAssignment(); 
        if (stmt) return stmt;

        if (peek().type == TOK_EOF) return nullptr;

        consume();
        return nullptr;
    };

    auto parseUnknown = [&]() {
        auto typePa = parseType();
        // es un tipo
        if (typePa != VariablesSize::Misingno) {
            consume();
            auto varName = consume();
            std::cout << typePa << " " << varName.value << std::endl;
        }
        else if (peek().type == TOK_EOF) return 1;
        consume();
        return 0;
    };
    BodyCode program;

    while (peek().type != TOK_EOF) {
        auto stmt = parseStatement();
        if (stmt) {
            program.body.push_back(stmt);
        }
    }

    return program;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << argv[0] << " <*.c>\n";
        return 1;
    }

    std::string filename = argv[1];
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "!\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string code = buffer.str();

    BodyCode program = parseCode(code);
    std::cout << generateCode(program) << std::endl;
}