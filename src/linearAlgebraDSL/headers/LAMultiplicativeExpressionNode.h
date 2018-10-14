#ifndef LA_MULTIPLICATIVEEXPRESSION_NODE_H
#define LA_MULTIPLICATIVEEXPRESSION_NODE_H

// by Binhang, June 2017

#include "LAASTNode.h"
#include "LAExpressionNode.h"
#include "LAPostfixExpressionNode.h"


struct LAMultiplicativeExpressionNode;
typedef std::shared_ptr<struct LAMultiplicativeExpressionNode> LAMultiplicativeExpressionNodePtr;


struct LAMultiplicativeExpressionNode : public LAExpressionNode {

private:
    std::string multiOperator;
    LAMultiplicativeExpressionNodePtr leftChild = NULL;
    LAPostfixExpressionNodePtr rightChild = NULL;
    LAMultiplicativeExpressionNodePtr me = NULL;
    pdb::Handle<pdb::Computation> query1;
    pdb::Handle<pdb::Computation> query2;
    LADimension dim;

public:
    LAMultiplicativeExpressionNode(const char* op)
        : LAExpressionNode(LA_ASTNODE_TYPE_MULTIPLICATIVEEXPRESSION) {
        multiOperator = op;
    }

    std::string toString() final {
        if (multiOperator.compare("none") == 0) {
            return rightChild->toString();
        } else if (multiOperator.compare("multiply") == 0) {
            return leftChild->toString() + " %*% " + rightChild->toString();
        } else if (multiOperator.compare("transpose_multiply") == 0) {
            return leftChild->toString() + " '* " + rightChild->toString();
        } else if (multiOperator.compare("scale_multiply") == 0) {
            return leftChild->toString() + " * " + rightChild->toString();
        } else {
            return "MultiplicativeExpression invalid operator: " + multiOperator;
        }
    }

    pdb::Handle<pdb::Computation>& evaluate(LAPDBInstance& instance) final;

    void setShared(LAMultiplicativeExpressionNodePtr meIn) {
        me = meIn;
    }

    void setLeftChild(LAMultiplicativeExpressionNodePtr lptr) {
        leftChild = lptr;
    }

    void setRightChild(LAPostfixExpressionNodePtr rptr) {
        rightChild = rptr;
    }

    bool isSyntaxSugarInitializer() {
        if (multiOperator.compare("none") == 0) {
            return rightChild->isSyntaxSugarInitializer();
        } else {
            return false;
        }
    }

    LADimension getDimension() {
        return dim;
    }

    void setDimension(LADimension other) {
        dim = other;
    }
};

#endif