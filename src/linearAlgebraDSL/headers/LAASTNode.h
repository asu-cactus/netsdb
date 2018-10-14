#ifndef LA_ASTNODE_H
#define LA_ASTNODE_H

#define LA_ASTNODE_TYPE_IDENTIFIER 1
#define LA_ASTNODE_TYPE_INITIALIZER 2
#define LA_ASTNODE_TYPE_PRIMARYEXPRESSION 3
#define LA_ASTNODE_TYPE_POSTFIXEXPRESSION 4
#define LA_ASTNODE_TYPE_MULTIPLICATIVEEXPRESSION 5
#define LA_ASTNODE_TYPE_ADDITIVEEXPRESSION 6
#define LA_ASTNODE_TYPE_STATEMENT 7

#include <iostream>
#include <memory>

// by Binhang, June 2017

struct LAASTNode;
typedef std::shared_ptr<struct LAASTNode> LAASTNodePtr;


struct LAASTNode {
private:
    int type;

protected:
    LAASTNode(int t) : type(t) {}

public:
    virtual std::string toString() = 0;

    int getType() {
        return type;
    }
};

#endif