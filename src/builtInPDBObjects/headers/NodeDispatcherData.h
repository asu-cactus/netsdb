//
// Created by Joseph Hwang on 9/13/16.
//

#ifndef OBJECTQUERYMODEL_NODEDISPATCHERDATA_H
#define OBJECTQUERYMODEL_NODEDISPATCHERDATA_H

#include "Object.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "DataTypes.h"
//#include "SetInfo.h"

#include <string>

// PRELOAD %NodeDispatcherData%

namespace pdb {

class NodeDispatcherData : public Object {

public:
    ENABLE_DEEP_COPY

    NodeDispatcherData() {}
    ~NodeDispatcherData() {}

    NodeDispatcherData(NodeID nodeId, int port, std::string address) {
        this->nodeId = nodeId;
        this->port = port;
        this->address = address;
        this->totalBytesSent = 0;
        this->totalBytesStored = 0;
    }

    NodeID getNodeId() {
        return this->nodeId;
    }

    int getPort() const {
        return this->port;
    }

    String getAddress() const {
        return this->address;
    }

    size_t getBytesSentToSet() const {
        return 0;
    }

    /*
    void addSet(std::string set, std::string database, long typeId) {
        sets.push_back(pdb::makeObject<pdb::SetInfo> (set, database, typeId));
        bytesSent.push_back(0);
    }
    */

    bool operator==(const NodeDispatcherData& other) {
        return this->port == other.getPort() && this->address == other.getAddress();
    }

    virtual std::string toString() override {
        return std::to_string(this->port) + ":" + static_cast<std::string>(this->address);
    }

private:
    NodeID nodeId;
    int port;
    String address;

    // TODO: Replace this with map when possible. Provides a mapping between a set and the number of
    // bytes sent to it.
    // Vector<Handle<SetInfo>> sets;

    // JOSEPH: These bottom values will only be available to the dispatcher
    Vector<size_t> bytesSent;
    Vector<size_t> bytesStored;

    size_t totalBytesSent;
    size_t totalBytesStored;

    // TODO: Add database and set information
};
}


#endif  // OBJECTQUERYMODEL_NODEDISPATCHERDATA_H
