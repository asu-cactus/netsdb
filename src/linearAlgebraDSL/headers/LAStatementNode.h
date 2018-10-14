#ifndef LA_STATEMENT_NODE_H
#define LA_STATEMENT_NODE_H

#include "LAASTNode.h"
#include "LAIdentifierNode.h"
#include "LAExpressionNode.h"
#include "LAIdentifierNode.h"
#include "LAInitializerNode.h"
#include "LAPrimaryExpressionNode.h"
#include "LAPostfixExpressionNode.h"
#include "LAMultiplicativeExpressionNode.h"
#include "LAAdditiveExpressionNode.h"

// by Binhang, June 2017

struct LAStatementNode;
typedef std::shared_ptr<struct LAStatementNode> LAStatementNodePtr;


struct LAStatementNode : public LAASTNode {

private:
    LAIdentifierNodePtr identifier = NULL;

    LAExpressionNodePtr expression = NULL;

    LAStatementNodePtr me = NULL;

    bool printQueryResult = true;


public:
    LAStatementNode() : LAASTNode(LA_ASTNODE_TYPE_STATEMENT) {}

    std::string toString() final {
        return identifier->toString() + " = " + expression->toString();
    }

    void setShared(LAStatementNodePtr meIn) {
        me = meIn;
    }

    void setLeftIdentifier(LAIdentifierNodePtr i) {
        identifier = i;
    }

    void setRightExpression(LAExpressionNodePtr e) {
        expression = e;
    }

    void evaluateQuery(LAPDBInstance& instance);
};

#endif