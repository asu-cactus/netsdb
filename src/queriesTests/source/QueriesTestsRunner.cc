#include "QueriesTestsRunner.h"
#include "SelectionTests.h"

namespace pdb_tests {
void runQueriesTests(UnitTest& qunit) {
    testSelectionExecute(qunit);
}
}
