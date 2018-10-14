#include "Store.h"

namespace pdb_detail {
Store::Store(TableColumns columnsToStore, const string& destination)
    : Instruction(InstructionType::store),
      columnsToStore(columnsToStore),
      destination(destination) {}

void Store::match(function<void(Load&)>,
                  function<void(ApplyFunction&)>,
                  function<void(ApplyMethod&)>,
                  function<void(Filter&)>,
                  function<void(Hoist&)>,
                  function<void(GreaterThan&)>,
                  function<void(Store&)> forStore) {
    forStore(*this);
}

StorePtr makeStore(TableColumns columnsToStore, const string& destination) {
    return make_shared<Store>(columnsToStore, destination);
}
}