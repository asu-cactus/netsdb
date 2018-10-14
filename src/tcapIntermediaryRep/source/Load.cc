#include "Load.h"

namespace pdb_detail {
Load::Load(TableColumn outputColumn, const string& source)
    : Instruction(InstructionType::load), outputColumn(outputColumn), source(source) {}

void Load::match(function<void(Load&)> forLoad,
                 function<void(ApplyFunction&)>,
                 function<void(ApplyMethod&)>,
                 function<void(Filter&)>,
                 function<void(Hoist&)>,
                 function<void(GreaterThan&)>,
                 function<void(Store&)>) {
    forLoad(*this);
}

LoadPtr makeLoad(const string& outputTableId, const string& outputColumnId, const string& source) {
    return make_shared<Load>(TableColumn(outputTableId, outputColumnId), source);
}
}