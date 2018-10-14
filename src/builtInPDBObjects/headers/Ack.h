
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_ACK_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_ACK_H_


#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

//  PRELOAD %Ack%


namespace pdb {
// this object type is sent from the server to client to acknowledge receiving a request.
class Ack : public Object {

public:
    Ack() {}

    ~Ack() {}

    String& getInfo() {
        return info;
    }

    void setInfo(String& info) {
        this->info = info;
    }

    bool getWasError() {
        return wasError;
    }

    void setWasError(bool wasError) {
        this->wasError = wasError;
    }

    ENABLE_DEEP_COPY


private:
    String info;
    bool wasError;
};
}


#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_ACK_H_ */
