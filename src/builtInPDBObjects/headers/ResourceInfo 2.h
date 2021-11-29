
#ifndef RESOURCE_INFO_H
#define RESOURCE_INFO_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "DataTypes.h"

//  PRELOAD %ResourceInfo%

namespace pdb {

/**
 * This class abstracts each node into a resource
 */
class ResourceInfo : public Object {

public:
    ResourceInfo() {}

    ~ResourceInfo() {}

    ResourceInfo(int numCores, int memSize, std::string address, int port, int nodeId)
        : numCores(numCores), memSize(memSize), address(address), port(port), nodeId(nodeId) {}

    // To get number of CPUs in this resource
    int getNumCores() {
        return this->numCores;
    }

    void setNumCores(int numCores) {
        this->numCores = numCores;
    }

    // To get size of memory in this resource
    int getMemSize() {
        return this->memSize;
    }

    void setMemSize(int memSize) {
        this->memSize = memSize;
    }


    String& getAddress() {
        return address;
    }

    void setAddress(pdb::String& address) {
        this->address = address;
    }

    NodeID getNodeId() {
        return this->nodeId;
    }

    void setNodeId(NodeID nodeId) {
        this->nodeId = nodeId;
    }

    int getPort() {

        return port;
    }

    void setPort(int port) {

        this->port = port;
    }

    ENABLE_DEEP_COPY

private:
    // number of CPU cores
    int numCores;

    // size of memory in MB
    int memSize;

    // hostname or IP address of the PDB server
    String address;

    // port of the PDB server
    int port;

    // NodeID of the PDB server
    NodeID nodeId;
};
}

#endif
