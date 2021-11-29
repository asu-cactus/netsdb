#ifndef LA_IDENTIFIER_NODE_H
#define LA_IDENTIFIER_NODE_H

#include "LAASTNode.h"
#include "LAExpressionNode.h"

// by Binhang, June 2017

struct LAIdentifierNode;
typedef std::shared_ptr<struct LAIdentifierNode> LAIdentifierNodePtr;


struct LAIdentifierNode : public LAExpressionNode {
private:
    std::string name;
    bool linked;
    pdb::Handle<pdb::Computation> scanSet;
    LAIdentifierNodePtr me;
    LADimension dim;

public:
    LAIdentifierNode(char* tag) : LAExpressionNode(LA_ASTNODE_TYPE_IDENTIFIER) {
        name = tag;
        linked = false;
    }

    std::string toString() final {
        return name;
    }

    pdb::Handle<pdb::Computation>& evaluate(LAPDBInstance& instance) final;

    void setScanSet(pdb::Handle<pdb::Computation> setMe) {
        linked = true;
        scanSet = setMe;
    }

    pdb::Handle<pdb::Computation> getScanSet() {
        return scanSet;
    }

    void setShared(LAIdentifierNodePtr meIn) {
        me = meIn;
    }

    bool isLinked() {
        return linked;
    }

    bool isSyntaxSugarInitializer() {
        return false;
    }

    void setDimension(LADimension other) {
        dim = other;
    }

    LADimension getDimension() {
        return dim;
    }
};

#endif