#ifndef LA_EXPRESSION_NODE_H
#define LA_EXPRESSION_NODE_H

#include "LAASTNode.h"
#include "Query.h"
#include "Lambda.h"
#include "LAPDBInstance.h"
#include "LADimension.h"

// by Binhang, June 2017

struct LAExpressionNode;
typedef std::shared_ptr<struct LAExpressionNode> LAExpressionNodePtr;


struct LAExpressionNode : public LAASTNode {
public:
    LAExpressionNode(int t) : LAASTNode(t) {}
    ~LAExpressionNode() {}
    virtual pdb::Handle<pdb::Computation>& evaluate(LAPDBInstance& instance) = 0;
    virtual std::string toString() = 0;
    virtual bool isSyntaxSugarInitializer() = 0;
    virtual LADimension getDimension() = 0;
};

#endif
