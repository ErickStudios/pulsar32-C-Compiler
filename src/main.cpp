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
struct Felix {
    std::map<std::string, VariablesSize>    variables;
    std::vector<StructCode*>                structs;
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
    Felix& fel,
    int tabs
) {
    std::string out;
    std::map<std::string, VariablesSize>& variables = fel.variables;
    std::string indent = std::string(tabs * 2, ' ');

    for (auto stmt : program.body) {

        if (auto var = dynamic_cast<VarDecl*>(stmt)) {

            variables[var->name] = var->size;

            out += var->name + ": ";
            out += "Assume-" + parseToSize(var->size) + " 0\n";
        }

        else if (auto structa = dynamic_cast<StructCode*>(stmt)) {
            fel.structs.push_back(structa);
        }

        else if (auto vactar = dynamic_cast<VectorInstanciated*>(stmt)) {
            out += vactar->name + ": ";

            out += "Assume-" + parseToSize(vactar->elementsSize) + " ";

            for (size_t idx = 0; idx < vactar->vinit.size(); ++idx) {
                out += std::to_string(vactar->vinit[idx]);
                
                if (idx + 1 < vactar->vinit.size()) {
                    out += ",";
                }
            }

            out += "\n";
        }

        else if (auto vari = dynamic_cast<VarDeclWithInit*>(stmt)) {

            variables[vari->name] = vari->size;

            out += vari->name + ": ";
            out += "Assume-" + parseToSize(vari->size) + " " + vari->init_val + "\n";
        }

        else if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {

            out += fn->nam + ":\n";

            if (fn->code) {
                out += generateCode(*fn->code, fel, tabs + 1);
            }

        }

        else if (auto asg = dynamic_cast<Assignment*>(stmt)) {

            VariablesSize size = variables[asg->name];

            out += indent + "Lea-Dword " + asg->name + "\n";

            out += indent + "Out-" +
                   parseToSize(size) +
                   " " +
                   asg->value +
                   "\n";
        }

        else if (auto bin = dynamic_cast<BinaryOperation*>(stmt)) {

            VariablesSize size = variables[bin->dest];

            std::string opName;

            out += indent + "Lea-Dword " + bin->right + "\n";
            out += indent + "Mov-" + parseToSize(size) + "\n";
            out += indent + "Push-" + parseToSize(size) +  " Out\n";

            out += indent + "Lea-Dword " + bin->left + "\n";
            out += indent + "Mov-" + parseToSize(size) + "\n";
            out += indent + "Push-" + parseToSize(size) + " Out\n";

            if (bin->op == "+") opName = "Add";
            else if (bin->op == "-") opName = "Sub";
            else if (bin->op == "*") opName = "Mul";
            else if (bin->op == "/") opName = "Div";
            else if (bin->op == "&") opName = "And";
            else if (bin->op == "|") opName = "Or";
            else if (bin->op == "^") opName = "Xor";

            out += indent + opName +
                "-" +
                parseToSize(size) +
                "-Sp-Sp\n";

            out += indent + "Lea-Dword " + bin->dest + "\n";

            out += indent + "Out-" +
                parseToSize(size) +
                " Out\n";
        }
    
        else if (auto jmp_t = dynamic_cast<JmpSafeAsm*>(stmt)) {

            out += indent + "Jmp-Dword-Clasic " + jmp_t->dest + "\n";

        }

        else if (auto call_t = dynamic_cast<CallSafeAsm*>(stmt)) {

            out += indent + "Jmp-Dword-Call " + call_t->dest + "\n";

        }

        else if (auto ret_t = dynamic_cast<RetSafeAsm*>(stmt)) {
            out += indent + "Jmp-Dword-Clasic Sp\n";
        }
    }

    return out;
}
std::string generateCode(
    BodyCode& program,
    Felix& fel
)
{
    return generateCode(program, fel, 0);
}
BodyCode parseCode(const std::string& code) {
    std::map<std::string, Lex> types;
    std::function<StatmentNode*()> parseStatement;

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
    auto getVectorialInitDot = [&]() -> std::vector<int> {
        std::vector<int> vinit;
        if (peek().type == TOK_STRING) {
            std::string literal = consume().value; 
            
            for (char c : literal) {
                vinit.push_back(static_cast<int>(c));
            }
        }
        else if (peek().type == TOK_NUMBER) {
            vinit.push_back(std::stoi(consume().value));
        }

        return vinit;
    };
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
        if (peek().type == TOK_IDENTIFIER && peek().value == "ptr_t") {
            consume();
            return VariablesSize::Int;
        }

        return VariablesSize::Misingno;
    };
    auto parseVectorInitializer = [&]() -> StatmentNode* {
        if (peek().value == "vector_t") {
            consume();
            expect("(");
            auto ty = parseType();
            if (ty == VariablesSize::Misingno) return nullptr;
            expect(")");
            auto name = consume().value;
            auto vector_ta = new VectorInstanciated();
            vector_ta->name = name;
            expect("(");
            while (peek().value != ")") {
                auto mm = getVectorialInitDot();
                vector_ta->vinit.insert(vector_ta->vinit.end(), mm.begin(), mm.end());
                if (peek().value == ",") {
                    consume();
                    continue;
                } 
                else if (peek().value == ")") break;
                consume();
            }

            expect(")");

            expect(";");

            return vector_ta;
        }

        return nullptr;
    };
    auto parseAssignment = [&]() -> StatmentNode* {
        if (peek().type == TOK_IDENTIFIER) {
            std::string name = consume().value;

            if (peek().value == "=") {
                consume();

                auto value = consume().value;

                expect(";");

                auto assign = new Assignment();
                assign->name = name;
                assign->value = value;

                return assign;
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
    auto parseStruct = [&]() -> StatmentNode* { 
        if (peek().value == "struct") {
            consume();
            auto name = consume().value;
            expect("{");
            auto strct = new StructCode();
            strct->name = name;
            while (true) {
                if (peek().value == "}") {
                    consume();
                    expect(";");
                    break;
                }

                auto type = parseType();

                if (type == VariablesSize::Misingno) return nullptr;

                auto name2 = expect(peek().value).value;
                expect(";");
                auto nm = new StructField();
                nm->name = name2;
                nm->size = type;
                strct->body.push_back(nm);
            }
            return strct;
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

        stmt = parseVectorInitializer();
        if (stmt) return stmt;

        stmt = parseStruct();
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

    Felix env;
    std::string hdr = "; used files: ";

    std::string cdm = "";

    for (int i = 1; i < argc; ++i) {
        std::string filename = argv[i];
        hdr += filename + " ";
        std::ifstream file(filename);

        if (!file) {
            std::cerr << "Error: No se pudo abrir " << filename << "\n";
            continue;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        BodyCode program = parseCode(code);
        
        cdm += generateCode(program, env) + "\n";
    }

    std::cout << (hdr + "\n" + cdm) << std::endl;
    
    return 0;
}