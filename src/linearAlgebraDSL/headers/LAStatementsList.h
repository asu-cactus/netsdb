#ifndef LA_STATEMENT_LIST_H
#define LA_STATEMENT_LIST_H


#include <vector>
#include "LAStatementNode.h"

// by Binhang, June 2017

struct LAStatementsList {

private:
    std::vector<LAStatementNodePtr> statements;

public:
    void addStatement(LAStatementNodePtr addMe) {
        statements.push_back(addMe);
    }

    int size() {
        return statements.size();
    }

    LAStatementNodePtr get(int i) {
        return statements[i];
    }
};

#endif