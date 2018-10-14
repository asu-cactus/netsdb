#include "TcapTestsRunner.h"

#include "ParseTcapTests.h"

namespace pdb_tests {
void runTcapParserTests(UnitTest& qunit) {
    testParseTcap1(qunit);
}
}
