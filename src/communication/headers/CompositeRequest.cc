
#ifndef COMPOSITE_REQUEST_CC
#define COMPOSITE_REQUEST_CC

#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"

namespace pdb {

template <class RequestType, class ResponseType, class ReturnType, class... RequestTypeParams>
ReturnType compositeRequest(
    PDBLoggerPtr myLogger,
    int port,
    std::string address,
    ReturnType onErr,
    size_t bytesForRequest,
    function<ReturnType(Handle<ResponseType>, PDBCommunicator)> processResponse,
    RequestTypeParams&&... args) {

    PDBCommunicator temp;
    string errMsg;
    bool success;

    if (temp.connectToInternetServer(myLogger, port, address, errMsg)) {
        myLogger->error(errMsg);
        myLogger->error("compositeRequest: not able to connect to server.\n");
        return onErr;
    }

    // build the request
    const UseTemporaryAllocationBlock tempBlock{bytesForRequest};
    Handle<RequestType> request = makeObject<RequestType>(args...);
    ;
    if (!temp.sendObject(request, errMsg)) {
        myLogger->error(errMsg);
        myLogger->error("compositeRequest: not able to send request to server.\n");
        return onErr;
    }

    // get the response and process it
    ReturnType finalResult;
    void* memory = malloc(temp.getSizeOfNextObject());
    if (memory == nullptr) {
        std::cout << "CompositeRequest.cc: Failed to allocate memory"<< std::endl;
        exit(1);
    }
    {
        Handle<ResponseType> result = temp.getNextObject<ResponseType>(memory, success, errMsg);
        if (!success) {
            myLogger->error(errMsg);
            myLogger->error("compositeRequest: not able to get next object over the wire.\n");
            return onErr;
        }

        finalResult = processResponse(result, temp);
    }
    free(memory);
    return finalResult;
}
}
#endif
