#ifndef PDB_QUERYINTERMEDIARYREP_SETEXPRESSIONIRALGO_H
#define PDB_QUERYINTERMEDIARYREP_SETEXPRESSIONIRALGO_H

namespace pdb_detail {
class ProjectionIr;  // without forward declaration we get circular includes and can't compile

class SelectionIr;  // without forward declaration we get circular includes and can't compile

class SourceSetNameIr;  // without forward declaration we get circular includes and can't compile

/**
 * Visitor interface for Visitor Pattern over SetExpressionIr.
 */
class SetExpressionIrAlgo {

public:
    /**
     * For the SelectionIr case.
     */
    virtual void forProjection(ProjectionIr& projection) = 0;

    /**
     * For the SelectionIr case.
     */
    virtual void forSelection(SelectionIr& selection) = 0;

    /**
     * For the SourceSetNameIr case.
     */
    virtual void forSourceSetName(SourceSetNameIr& setName) = 0;
};
}

#endif  // PDB_QUERYINTERMEDIARYREP_SETEXPRESSIONIRALGO_H
