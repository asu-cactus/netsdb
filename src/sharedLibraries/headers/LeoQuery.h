#ifndef LEO_QUERY_H
#define LEO_QUERY_H

#include "BaseQuery.h"
#include "Supervisor.h"
#include "Employee.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

class LeoQuery : public BaseQuery {

public:
    ENABLE_DEEP_COPY

    LeoQuery() {}

    Lambda<bool> getSelection(Handle<Supervisor>& checkMe) {
        return makeLambdaFromMethod(checkMe, getSteve) == makeLambdaFromMember(checkMe, me);
    }

    Lambda<Handle<Employee>> getProjection(Handle<Supervisor>& checkMe) {
        return makeLambdaFromMethod(checkMe, getMe);
    }

    virtual void toMap(std::map<std::string, GenericLambdaObjectPtr>& fillMe,
                       int& identifier) override {
        Handle<Supervisor> temp = nullptr;
        getSelection(temp).toMap(fillMe, identifier);
        getProjection(temp).toMap(fillMe, identifier);
    }
};

#endif
