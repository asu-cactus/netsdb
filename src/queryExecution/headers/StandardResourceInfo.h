
#ifndef STANDARD_RESOURCE_INFO_H
#define STANDARD_RESOURCE_INFO_H

#include <memory>
namespace pdb {

class StandardResourceInfo;
typedef std::shared_ptr<StandardResourceInfo> StandardResourceInfoPtr;


/**
 * This class abstracts each node into a resource
 */
class StandardResourceInfo {

public:
    StandardResourceInfo() {}

    ~StandardResourceInfo() {}

    StandardResourceInfo(int numCores, int memSize, std::string address, int port, int nodeId)
        : numCores(numCores), memSize(memSize), address(address), port(port), nodeId(nodeId) {}

    // To get number of CPUs in this resource
    // <=0 for unknown
    int getNumCores() {
        return this->numCores;
    }
    // <=0 for unknown
    void setNumCores(int numCores) {
        this->numCores = numCores;
    }

    // To get size of memory in this resource
    // <= 0 for unknown
    int getMemSize() {
        return this->memSize;
    }
    // <= 0 for unknown
    void setMemSize(int memSize) {
        this->memSize = memSize;
    }


    std::string& getAddress() {
        return address;
    }

    void setAddress(std::string& address) {
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


private:
    // number of CPU cores
    int numCores;

    // size of memory in MB
    int memSize;

    // hostname or IP address of the PDB server
    std::string address;

    // port of the PDB server
    int port;

    // NodeID of the PDB server
    NodeID nodeId;
};
}

#endif
