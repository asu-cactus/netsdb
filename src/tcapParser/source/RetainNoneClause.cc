#include "RetainNoneClause.h"

namespace pdb_detail {
bool RetainNoneClause::isAll() {
    return false;
}

bool RetainNoneClause::isNone() {
    return true;
}

void RetainNoneClause::match(function<void(RetainAllClause&)>,
                             function<void(RetainExplicitClause&)>,
                             function<void(RetainNoneClause&)> forNone) {
    forNone(*this);
}
}
