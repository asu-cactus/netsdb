
#ifndef FILTER_QUERY_EXEC_H
#define FILTER_QUERY_EXEC_H

#include "QueryExecutor.h"
#include "TupleSetMachine.h"
#include "TupleSet.h"
#include <vector>

namespace pdb {

// runs a filter operation
class FilterQueryExecutor : public QueryExecutor {

private:
    // this is the output TupleSet that we return
    TupleSetPtr output;

    // the attribute to operate on
    int whichAtt;

    // to setup the output tuple set
    TupleSetSetupMachine<bool> myMachine;

public:
    FilterQueryExecutor(TupleSpec& inputSchema,
                        TupleSpec& attsToOperateOn,
                        TupleSpec& attsToIncludeInOutput)
        : myMachine(inputSchema, attsToIncludeInOutput) {

        output = std::make_shared<TupleSet>();

        // this is the input attribute that we will process
        std::vector<int> matches = myMachine.match(attsToOperateOn);
        whichAtt = matches[0];
    }

    TupleSetPtr process(TupleSetPtr input) override {

        // set up the output tuple set
        myMachine.setup(input, output);

        // get the input column to use as a filter
        std::vector<bool>& inputColumn = input->getColumn<bool>(whichAtt);

        // loop over the columns and filter
        int numColumns = output->getNumColumns();
        for (int i = 0; i < numColumns; i++) {
            output->filterColumn(i, inputColumn);
        }

        return output;
    }
};
}

#endif
