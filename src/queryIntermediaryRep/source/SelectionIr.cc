
#include <memory>

#include "ProcessorFactoryFilterQueryProcessor.h"
#include "SelectionIr.h"

#include "Selection.h"
#include "InterfaceFunctions.h"

using std::make_shared;

using pdb::FilterQueryProcessor;
using pdb::makeObject;
using pdb::ProcessorFactoryFilterQueryProcessor;
using pdb::Selection;

namespace pdb_detail {

SelectionIr::SelectionIr() {}

SelectionIr::SelectionIr(shared_ptr<SetExpressionIr> inputSet, Handle<QueryBase> originalSelection)
    : _inputSet(inputSet), _originalSelection(originalSelection) {}

string SelectionIr::getName() {
    return "SelectionIr";
}

void SelectionIr::execute(SetExpressionIrAlgo& algo) {
    algo.forSelection(*this);
}

shared_ptr<SetExpressionIr> SelectionIr::getInputSet() {
    return _inputSet;
}

Handle<ProcessorFactory> SelectionIr::makeProcessorFactory() {
    return makeObject<ProcessorFactoryFilterQueryProcessor>(_originalSelection);
}

SimpleSingleTableQueryProcessorPtr SelectionIr::makeProcessor() {
    return make_shared<FilterQueryProcessor<Object, Object>>(_originalSelection);
}

Handle<QueryBase> SelectionIr::getQueryBase() {
    return _originalSelection;
}
}
