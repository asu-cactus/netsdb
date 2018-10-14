#include "TableAssignment.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {
TableAssignment::TableAssignment(TcapIdentifier tableName,
                                 shared_ptr<vector<TcapIdentifier>> columnNames,
                                 shared_ptr<TableExpression> value)
    : TableAssignment(make_shared<vector<TcapAttribute>>(), tableName, columnNames, value) {
    if (attributes == nullptr)
        throw invalid_argument("attributes may not be null");

    if (columnNames == nullptr)
        throw invalid_argument("columnsToStore may not be null");
}

TableAssignment::TableAssignment(shared_ptr<vector<TcapAttribute>> attributes,
                                 TcapIdentifier tableName,
                                 shared_ptr<vector<TcapIdentifier>> columnNames,
                                 shared_ptr<TableExpression> value)
    : TcapStatement(attributes), tableName(tableName), columnNames(columnNames), value(value) {
    if (attributes == nullptr)
        throw invalid_argument("attributes may not be null");

    if (columnNames == nullptr)
        throw invalid_argument("columnsToStore may not be null");
}

TableAssignment::TableAssignment(TcapAttribute attribute,
                                 TcapIdentifier tableName,
                                 shared_ptr<vector<TcapIdentifier>> columnNames,
                                 shared_ptr<TableExpression> value)
    : TcapStatement(attribute), tableName(tableName), columnNames(columnNames), value(value) {}

void TableAssignment::match(function<void(TableAssignment&)> forTableAssignment,
                            function<void(StoreOperation&)>) {
    forTableAssignment(*this);
}
}