
#ifndef OBJECTQUERYMODEL_NODEPARTITIONDATA_H
#define OBJECTQUERYMODEL_NODEPARTITIONDATA_H

#include "DataTypes.h"
#include "NodeDispatcherData.h"

#include <string>

namespace pdb {

class NodePartitionData;
typedef std::shared_ptr<NodePartitionData> NodePartitionDataPtr;

class NodePartitionData {
public:
    NodePartitionData(NodeID nodeId,
                      int port,
                      std::string address,
                      std::pair<std::string, std::string> setAndDatabaseName);

    std::string toString() const;
    NodeID getNodeId() const;
    int getPort() const;
    std::string getAddress() const;
    std::string getSetName() const;
    std::string getDatabaseName() const;
    size_t getTotalBytesSent() const;

    bool operator==(const NodePartitionData& other) {
        return this->port == other.getPort() && this->address == other.getAddress();
    }

private:
    NodeID nodeId;
    int port;
    std::string address;
    std::string setName;
    std::string databaseName;

    size_t totalBytesSent;
};
}


#endif  // OBJECTQUERYMODEL_NODEPARTITIONDATA_H
