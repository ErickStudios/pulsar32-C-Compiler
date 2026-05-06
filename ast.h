#pragma once
#include <string>
#include <vector>

struct Node;
struct StatmentNode;
struct BodyCode;

enum VariablesSize {
    Char,
    Short,
    Int,
    CharSize = 1,
    ShortSize = 2,
    IntSize = 4,
    Misingno = -1
};

struct Node {
    virtual ~Node() = default;
};

struct StatmentNode : Node {
    virtual std::string getType() const = 0;
};

struct VarDecl : StatmentNode {
    std::string name;
    VariablesSize size;
    std::string getType() const override {
        return "VarDecl";
    }
};

struct Assignment : StatmentNode {
    std::string name;
    int value;

    std::string getType() const override {
        return "Assignment";
    }
};

struct FunctionDecl : StatmentNode {
    std::string nam;
    BodyCode* code;
    VariablesSize size;
    std::string getType() const override {
        return "FunctionDecl";
    }
};

struct BodyCode : Node {
    std::vector<StatmentNode*> body;
};