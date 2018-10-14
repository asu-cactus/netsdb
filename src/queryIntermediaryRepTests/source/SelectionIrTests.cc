
#include "SelectionIrTests.h"

#include "Handle.h"
#include "SelectionIr.h"
#include "SetExpressionIr.h"
#include "QueryNodeIrAlgo.h"
#include "RecordPredicateIr.h"
#include "SourceSetNameIr.h"

using pdb::Handle;

using pdb_detail::SelectionIr;
using pdb_detail::SetExpressionIr;
using pdb_detail::RecordPredicateIr;
using pdb_detail::ProjectionIr;
using pdb_detail::SetExpressionIrAlgo;
using pdb_detail::SourceSetNameIr;

namespace pdb_tests {
void testSelectionIrExecute(UnitTest& qunit) {
    class Algo : public SetExpressionIrAlgo {
    public:
        void forProjection(ProjectionIr& projection) {}

        void forSelection(SelectionIr& selection) {
            success = true;
        }

        void forSourceSetName(SourceSetNameIr& setName) {}

        bool success = false;
    } algo;

    shared_ptr<SetExpressionIr> nullInputSet;
    // shared_ptr<RecordPredicateIr> nullCondition;
    Handle<Selection<Object, Object>> nullSelection;

    SelectionIr selection(nullInputSet, nullSelection);

    selection.execute(algo);


    QUNIT_IS_TRUE(algo.success);
}
}