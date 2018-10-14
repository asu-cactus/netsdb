#include "StoreOperation.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {

StoreOperation::StoreOperation(TcapIdentifier outputTable,
                               shared_ptr<vector<TcapIdentifier>> columnsToStore,
                               const string& destination)
    : StoreOperation(
          make_shared<vector<TcapAttribute>>(), outputTable, columnsToStore, destination) {
    if (columnsToStore == nullptr)
        throw invalid_argument("columnsToStore may not be null");
}

StoreOperation::StoreOperation(shared_ptr<vector<TcapAttribute>> attributes,
                               TcapIdentifier outputTable,
                               shared_ptr<vector<TcapIdentifier>> columnsToStore,
                               const string& destination)
    : TcapStatement(attributes),
      outputTable(outputTable),
      columnsToStore(columnsToStore),
      destination(destination) {

    if (columnsToStore == nullptr)
        throw invalid_argument("columnsToStore may not be null");
}

void StoreOperation::match(function<void(TableAssignment&)>,
                           function<void(StoreOperation&)> forStore) {
    forStore(*this);
}
}