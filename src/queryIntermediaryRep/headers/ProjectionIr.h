#ifndef PDB_QUERYINTERMEDIARYREP_PROJECTION_H
#define PDB_QUERYINTERMEDIARYREP_PROJECTION_H

#include <memory>

#include "ProcessorFactory.h"
#include "QueryBase.h"
#include "Handle.h"
#include "SetExpressionIr.h"

using std::shared_ptr;

using pdb::Handle;
using pdb::ProcessorFactory;
using pdb::QueryBase;

namespace pdb_detail {
/**
 * A set where each element is transformation of an input set element.
 *
 * I.e. if the input set is is { a, b, c } the projection set is { f(x), f(b), f(c) }.
 *
 * Note that for the purposes of PDB sets no two objects are ever equivalent. So if the input set
 * has cardinality n the projection also has cardinatlity n because f(a) != f(a) for the purposes
 * of building this set.
 */
class ProjectionIr : public SetExpressionIr {

public:
    /**
     * Constructs a new ProjectionIr.
     *
     * @param inputSet the elements to be transformed by the projection
     * @param originalSelection the original user query node representation of the projection
     * @return a new ProjectionIr
     */
    ProjectionIr(shared_ptr<SetExpressionIr> inputSet, Handle<QueryBase> originalSelection);

    /**
     * @return the constant PROJECTION_IR
     */
    string getName() override;

    /**
     * Visitation hook in visitor pattern.
     */
    void execute(SetExpressionIrAlgo& algo) override;

    /**
     * @return the input set to the projection
     */
    shared_ptr<SetExpressionIr> getInputSet();

    // added for temporary use in physical planning
    Handle<QueryBase> getQueryBase();

    /**
     * @return create a new factory capabily of producing a SimpleSingleTableQueryProcessorPtr for
     * processing elements of the set.
     */
    Handle<ProcessorFactory> makeProcessorFactory();

    /**
     * The string "ProjectionIr"
     */
    static const string PROJECTION_IR;

private:
    /**
     * The input set to the projection.
     */
    shared_ptr<SetExpressionIr> _inputSet;

    /**
     * The original pdb::Selection<?,?> the projection mirrors.
     */
    Handle<QueryBase> _originalSelection;
};

typedef shared_ptr<ProjectionIr> ProjectionIrPtr;
}

#endif  // PDB_QUERYINTERMEDIARYREP_PROJECTION_H
