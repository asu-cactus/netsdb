/*
 * NodeInfo.h
 *
 *  Created on: Mar 7, 2016
 *      Author: Kia
 */

#ifndef NODE_INFO_H
#define NODE_INFO_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

//  PRELOAD %NodeInfo%

namespace pdb {

/**
 * This class encapsulates data about each node. This data is a dynamic data that represents the
 * current node CPU load in addition to other static
 * information like hostname or host IP address and port.
 */
class NodeInfo : public Object {

public:
    NodeInfo() {}

    ~NodeInfo() {}

    // CPU load might be used later to know how a processing node is overloaded with tasks.
    int getCpuLoad() {
        return cpuLoad;
    }

    void setCpuLoad(int cpuLoad) {
        this->cpuLoad = cpuLoad;
    }

    String& getHostName() {
        return hostName;
    }

    void setHostName(pdb::String& hostName) {
        this->hostName = hostName;
    }

    int getPort() {
        return port;
    }

    void setPort(int port) {
        this->port = port;
    }

    ENABLE_DEEP_COPY

private:
    // hostname or IP address of the PDB server
    String hostName;
    // port number on which the PDB server is running
    int port;

    // current cpu load of the server as an integer between 0-100, 100 means %100 CPU load.
    int cpuLoad;
};
}

#endif
