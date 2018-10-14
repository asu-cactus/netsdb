#include "QueriesTestsRunner.h"

#include "BuildIrTests.h"
#include "DotBuilderTests.h"
#include "ProjectionIrTests.h"
#include "SelectionIrTests.h"

namespace pdb_tests {
void runQueryIrTests(UnitTest& qunit) {
    testDotBuilderSelection(qunit);
    testBuildIrSelection1(qunit);
    testBuildIrSelection2(qunit);
    testBuildIrSelection3(qunit);
    testProjectionIrExecute(qunit);
    testSelectionIrExecute(qunit);
}
}
