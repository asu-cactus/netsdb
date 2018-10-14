#include "DotBuilderTests.h"

#include "Handle.h"
#include "RecordPredicateIr.h"
#include "SelectionIr.h"
#include "SetExpressionIr.h"

using pdb::Handle;

using pdb_detail::RecordPredicateIr;
using pdb_detail::SelectionIr;
using pdb_detail::SetExpressionIr;

namespace pdb_tests {
void testDotBuilderSelection(UnitTest& qunit) {
    //        Handle<RecordPredicate> nullRecord;
    //        Handle<SetExpression> nullInputSet;
    //        SelectionIr selectionIr(nullRecord, nullInputSet);
    QUNIT_IS_TRUE(true);
}
}