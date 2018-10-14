
#ifndef QUERIES_AND_PLAN_H
#define QUERIES_AND_PLAN_H

#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "BaseQuery.h"

// PRELOAD %QueriesAndPlan%

namespace pdb {

class QueriesAndPlan : public Object {

public:
    QueriesAndPlan() {}

    void setPlan(String myPlan) {
        plan = myPlan;
    }

    String getPlan() {
        return plan;
    }

    Handle<Vector<Handle<BaseQuery>>> getQueries() {
        return queries;
    }

    void addQuery(Handle<BaseQuery> query) {
        if (queries == nullptr) {
            queries = makeObject<Vector<Handle<BaseQuery>>>(1);
        }
        queries->push_back(query);
    }

    ENABLE_DEEP_COPY

private:
    String plan;

    // The queries to run the plan
    Handle<Vector<Handle<BaseQuery>>> queries;
};
}

#endif
