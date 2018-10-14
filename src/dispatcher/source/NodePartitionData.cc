#ifndef OBJECTQUERYMODEL_NODEPARTITIONDATA_CC
#define OBJECTQUERYMODEL_NODEPARTITIONDATA_CC

#include "PDBDebug.h"
#include "NodePartitionData.h"

namespace pdb {

NodePartitionData::NodePartitionData(NodeID nodeId,
                                     int port,
                                     std::string address,
                                     std::pair<std::string, std::string> setAndDatabaseName)
    : nodeId(nodeId),
      port(port),
      address(address),
      setName(setAndDatabaseName.first),
      databaseName(setAndDatabaseName.second) {


    PDB_COUT << "Constructing " << port << " and " << address << std::endl;

    PDB_COUT << "Now is " << getPort() << " and " << getAddress() << std::endl;

    PDB_COUT << "Now getting " << toString() << std::endl;

    this->totalBytesSent = 0;
}

std::string NodePartitionData::toString() const {
    return getAddress() + ":" + std::to_string(getPort());
}

NodeID NodePartitionData::getNodeId() const {
    return this->nodeId;
}
int NodePartitionData::getPort() const {
    return this->port;
}
std::string NodePartitionData::getAddress() const {
    return this->address;
}
std::string NodePartitionData::getSetName() const {
    return this->setName;
}
std::string NodePartitionData::getDatabaseName() const {
    return this->databaseName;
}
size_t NodePartitionData::getTotalBytesSent() const {
    return this->totalBytesSent;
}
}

#endif
