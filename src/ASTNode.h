#ifndef ASTNODE_H
#define ASTNODE_H

#include <memory>
#include <string>
#include <vector>

struct ASTNode {
    std::string type;
    std::string value;
    int line;
    int column;
    std::vector<std::unique_ptr<ASTNode>> children;

    static std::unique_ptr<ASTNode> make(const std::string& type,
                                          const std::string& value = "",
                                          int line = 0, int column = 0) {
        auto node = std::make_unique<ASTNode>();
        node->type = type;
        node->value = value;
        node->line = line;
        node->column = column;
        return node;
    }

    void add(std::unique_ptr<ASTNode> child) {
        if (child) {
            children.push_back(std::move(child));
        }
    }
};

std::string formatAST(const ASTNode& root, int indent = 0);

#endif
