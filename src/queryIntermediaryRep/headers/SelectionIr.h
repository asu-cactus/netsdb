#ifndef PDB_QUERYINTERMEDIARYREP_SELECTION_H
#define PDB_QUERYINTERMEDIARYREP_SELECTION_H

#include "Handle.h"
#include "ProcessorFactory.h"
#include "RecordPredicateIr.h"
#include "Selection.h"
#include "SetExpressionIr.h"
#include "SimpleSingleTableQueryProcessor.h"

using std::function;
using std::shared_ptr;

using pdb::Handle;
using pdb::ProcessorFactory;
using pdb::QueryBase;
using pdb::Selection;
using pdb::SimpleSingleTableQueryProcessorPtr;

namespace pdb_detail {
/**
 * An operation that produces a subset of a given set of "input" records by applying a given boolean
 * condition to
 * each record of the input set. Only those input records that that pass the given condition are
 * retained in the
 * produced subset.
 */
class SelectionIr : public SetExpressionIr {
public:
    SelectionIr();

    /**
     * Creates a selection over the given input set filtered by the given condition.
     *
     * @param inputSet the source of records from which the selection subset will be drawn.
     * @param condition the filter to apply to the input set to produce the subset
     * @param the page processor originally associated with user query node
     * @return the selection
     */
    SelectionIr(shared_ptr<SetExpressionIr> inputSet, Handle<QueryBase> originalSelection);

    // contract from super
    virtual void execute(SetExpressionIrAlgo& algo) override;

    /**
     * @return the set of input records from which the selection subset is drawn.
     */
    virtual shared_ptr<SetExpressionIr> getInputSet();

    Handle<ProcessorFactory> makeProcessorFactory();

    /**
     * @return gets a processor capable of applying this node type to input pages to produce output
     * pages.
     */
    SimpleSingleTableQueryProcessorPtr makeProcessor();

    string getName() override;

    // added for temporary use in physical planning
    Handle<QueryBase> getQueryBase();


private:
    /**
     * Records source.
     */
    shared_ptr<SetExpressionIr> _inputSet;

    /**
     * Records filter.
     */
    // TODO: generalize this to RecordPredicateIr
    Handle<QueryBase> _originalSelection;
};

typedef shared_ptr<SelectionIr> SelectionIrPtr;
}

#endif  // PDB_QUERYINTERMEDIARYREP_SELECTION_H
