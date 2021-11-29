
#ifndef QUERY_OUTPUT_H
#define QUERY_OUTPUT_H

#include "Query.h"
#include "OutputIterator.h"
#include "TypeName.h"
#include "SetScan.h"

// PRELOAD %QueryOutput <Nothing>%

namespace pdb {

template <class OutType>
class QueryOutput : public QueryBase {

public:
    ENABLE_DEEP_COPY

    QueryOutput(std::string& dbName, std::string& setName) {
        this->setSetName(setName);
        this->setdbName(dbName);
        myType = getTypeName<OutType>();
    }

    QueryOutput() {
        myType = getTypeName<OutType>();
    }

    QueryOutput(std::string dbName, std::string setName, Handle<QueryBase> input) {
        this->setSetName(setName);
        this->setDBName(dbName);
        myType = getTypeName<OutType>();
        this->setInput(input);
    }

    ~QueryOutput() {}

    virtual std::string getIthInputType(int i) override {
        if (i != 0) {
            return "Bad index";
        }
        return myType;
    }

    virtual std::string getOutputType() override {
        return "";
    }

    virtual int getNumInputs() override {
        return 1;
    }

    virtual std::string getQueryType() override {
        return "localoutput";
    }


    void match(function<void(QueryBase&)> forSelection,
               function<void(QueryBase&)> forSet,
               function<void(QueryBase&)> forQueryOutput) override {
        forQueryOutput(*this);
    };

private:
    // records the output type
    String myType;
};
}

#endif
