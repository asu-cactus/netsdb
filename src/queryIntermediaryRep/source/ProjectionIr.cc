#include "ProjectionIr.h"

#include "InterfaceFunctions.h"
#include "ProcessorFactoryProjectionQueryProcessor.h"

using pdb::makeObject;
using pdb::ProcessorFactoryProjectionQueryProcessor;

namespace pdb_detail {

const string ProjectionIr::PROJECTION_IR = "ProjectionIr";

ProjectionIr::ProjectionIr(shared_ptr<SetExpressionIr> inputSet,
                           Handle<QueryBase> originalSelection)
    : _inputSet(inputSet), _originalSelection(originalSelection) {}

string ProjectionIr::getName() {
    return PROJECTION_IR;
}

void ProjectionIr::execute(SetExpressionIrAlgo& algo) {
    algo.forProjection(*this);
}


shared_ptr<SetExpressionIr> ProjectionIr::getInputSet() {
    return _inputSet;
}


Handle<QueryBase> ProjectionIr::getQueryBase() {
    return _originalSelection;
}

Handle<ProcessorFactory> ProjectionIr::makeProcessorFactory() {
    return makeObject<ProcessorFactoryProjectionQueryProcessor>(_originalSelection);
}
}
