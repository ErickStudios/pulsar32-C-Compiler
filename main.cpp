#include <iostream>
#include "lexer.h"
#include "ast.h"
#include <functional>
#include <fstream>
#include <sstream>
#include <map>
struct Lex {
    VariablesSize size;
    std::string initializeWith;
};
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
std::string generateCode(
    BodyCode& program,
    std::map<std::string, VariablesSize>& variables
) {
    std::string out;

    for (auto stmt : program.body) {

        if (auto var = dynamic_cast<VarDecl*>(stmt)) {

            variables[var->name] = var->size;

            out += var->name + ": ";
            out += "Assume-" + parseToSize(var->size) + " 0\n";
        }

        else if (auto vari = dynamic_cast<VarDeclWithInit*>(stmt)) {

            variables[vari->name] = vari->size;

            out += vari->name + ": ";
            out += "Assume-" + parseToSize(vari->size) + " " + vari->init_val + "\n";
        }

        else if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {

            out += fn->nam + ":\n";

            if (fn->code) {
                out += generateCode(*fn->code, variables);
            }

        }

        else if (auto asg = dynamic_cast<Assignment*>(stmt)) {

            VariablesSize size = variables[asg->name];

            out += "Lea-Dword " + asg->name + "\n";

            out += "Out-" +
                   parseToSize(size) +
                   " " +
                   std::to_string(asg->value) +
                   "\n";
        }

        else if (auto bin = dynamic_cast<BinaryOperation*>(stmt)) {

            VariablesSize size = variables[bin->dest];

            std::string opName;

            out += "Lea-Dword " + bin->right + "\n";
            out += "Mov-" + parseToSize(size) + "\n";
            out += "Push-" + parseToSize(size) +  " Out\n";

            out += "Lea-Dword " + bin->left + "\n";
            out += "Mov-" + parseToSize(size) + "\n";
            out += "Push-" + parseToSize(size) + " Out\n";

            if (bin->op == "+") opName = "Add";
            else if (bin->op == "-") opName = "Sub";
            else if (bin->op == "*") opName = "Mul";
            else if (bin->op == "/") opName = "Div";
            else if (bin->op == "&") opName = "And";
            else if (bin->op == "|") opName = "Or";
            else if (bin->op == "^") opName = "Xor";

            out += opName +
                "-" +
                parseToSize(size) +
                "-Sp-Sp\n";

            out += "Lea-Dword " + bin->dest + "\n";

            out += "Out-" +
                parseToSize(size) +
                " Out\n";
        }
    
        else if (auto jmp_t = dynamic_cast<JmpSafeAsm*>(stmt)) {

            out += "Jmp-Dword-Clasic " + jmp_t->dest + "\n";

        }

        else if (auto call_t = dynamic_cast<CallSafeAsm*>(stmt)) {

            out += "Jmp-Dword-Call " + call_t->dest + "\n";

        }

        else if (auto ret_t = dynamic_cast<RetSafeAsm*>(stmt)) {
            out += "Jmp-Dword-Clasic Sp\n";
        }
    }

    return out;
}
BodyCode parseCode(const std::string& code) {
    std::map<std::string, Lex> types;

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
    auto parseVarInitDecl = [&]() -> StatmentNode* {
        if (types.find(peek().value) != types.end()) {
            auto typo = consume().value;
            expect("&");
            auto decl = consume();
            expect(";");
            auto vrl = new VarDeclWithInit();
            vrl->name = decl.value;
            vrl->size = types[typo].size;
            vrl->init_val = types[typo].initializeWith;
            return vrl;
        }
        return nullptr;
    };
    auto parseBinaryOperation = [&]() -> StatmentNode* {

        if (peek().type == TOK_IDENTIFIER) {

            std::string dest = consume().value;

            if (peek().value == ":") {

                consume();
                expect("=");

                std::string left = consume().value;

                std::string op = consume().value;

                std::string right = consume().value;

                expect(";");

                auto bin = new BinaryOperation();

                bin->dest = dest;
                bin->left = left;
                bin->right = right;
                bin->op = op;

                return bin;
            }

            i--;
        }

        return nullptr;
    };
    auto parseJmp = [&]() -> StatmentNode* {
        if (peek().value == "jmp_t") {
            consume();
            expect("(");
            auto dest = consume().value;
            expect(")");
            expect(";");
            auto jmp_t = new JmpSafeAsm();
            jmp_t->dest = dest;
            return jmp_t;
        }

        return nullptr;
    };
    auto parseCall = [&]() -> StatmentNode* {
        if (peek().value == "call_t") {
            consume();
            expect("(");
            auto dest = consume().value;
            expect(")");
            expect(";");
            auto call_t = new CallSafeAsm();
            call_t->dest = dest;
            return call_t;
        }

        return nullptr;
    };
    auto parseExternalTypedef = [&]() -> int {
        if (peek().value == "extern") {
            consume();
            auto ty = parseType();
            if (peek().value == "*") {
                consume();
                auto nameFunc = consume().value;
                expect("(");
                expect(")");
                expect(";");
                Lex myLex;
                myLex.size = ty;
                myLex.initializeWith = nameFunc;
                types[nameFunc + "_t"] = myLex;
                return 1;
            }
        }

        return 0;
    };
    parseStatement = [&]() -> StatmentNode* {
        auto stmt = parseJmp();
        if (stmt) return stmt;

        stmt = parseCall();
        if (stmt) return stmt;
        
        stmt = parseVarInitDecl();
        if (stmt) return stmt;

        stmt = parseVarDecl();
        if (stmt) return stmt;
        
        stmt = parseAssignment(); 
        if (stmt) return stmt;

        stmt = parseBinaryOperation();
        if (stmt) return stmt;

        auto stmt2 = parseExternalTypedef();
        if (stmt2 == 1) return nullptr;

        if (peek().value == "return") {
            consume();
            expect(";");
            auto ret_t = new RetSafeAsm();
            return ret_t;
        }

        if (peek().type == TOK_IDENTIFIER) {
            auto cm = consume();
            if (peek().type == TOK_RBRACE && peek().value == "(") {
                consume();
                expect(")");
                expect(";");
                auto call_t = new CallSafeAsm();
                call_t->dest = cm.value;
                return call_t;
            }
        }

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
        std::cerr << "Uso: " << argv[0] << " <archivo1.c> <archivo2.c> ...\n";
        return 1;
    }

    std::map<std::string, VariablesSize> vars;

    for (int i = 1; i < argc; ++i) {
        std::string filename = argv[i];
        std::ifstream file(filename);

        if (!file) {
            std::cerr << "Error: No se pudo abrir " << filename << "\n";
            continue;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        BodyCode program = parseCode(code);
        
        std::cout << generateCode(program, vars) << std::endl;
    }
    
    return 0;
}