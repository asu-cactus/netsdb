
#ifndef SIMPLE_SEND_BYTES_REQUEST_CC
#define SIMPLE_SEND_BYTES_REQUEST_CC

#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"

#ifndef MAX_RETRIES
#define MAX_RETRIES 5
#endif
#ifndef HEADER_SIZE
#define HEADER_SIZE 20
#endif

namespace pdb {

template <class RequestType, class ResponseType, class ReturnType, class... RequestTypeParams>
ReturnType simpleSendBytesRequest(PDBLoggerPtr myLogger,
                                  int port,
                                  std::string address,
                                  ReturnType onErr,
                                  size_t bytesForRequest,
                                  function<ReturnType(Handle<ResponseType>)> processResponse,
                                  char* bytes,
                                  size_t numBytes,
                                  RequestTypeParams&&... args) {

    int retries = 0;

    while (retries <= MAX_RETRIES) {

        PDBCommunicator temp;
        string errMsg;
        bool success;

        if (temp.connectToInternetServer(myLogger, port, address, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("simpleSendDataRequest: not able to connect to server.\n");
            std::cout << "ERROR: can't connect to remote server with port =" << port
                      << " and address =" << address << std::endl;
            return onErr;
        }

        // build the request
        if (bytesForRequest < HEADER_SIZE) {
            std::cout << "ERROR: block size is too small" << std::endl;
            return onErr;
        }
        const UseTemporaryAllocationBlock tempBlock{bytesForRequest};
        Handle<RequestType> request = makeObject<RequestType>(args...);
        if (!temp.sendObject(request, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("simpleSendDataRequest: not able to send request to server.\n");
            if (retries < MAX_RETRIES) {
                retries++;
                continue;
            } else {
                return onErr;
            }
        }
        // now, send the bytes
        if (!temp.sendBytes(bytes, numBytes, errMsg)) {
            myLogger->error(errMsg);
            myLogger->error("simpleSendDataRequest: not able to send data to server.\n");
            if (retries < MAX_RETRIES) {
                retries++;
                continue;
            } else {
                return onErr;
            }
        }

        // get the response and process it
        size_t objectSize = temp.getSizeOfNextObject();
        if (objectSize == 0) {
            myLogger->error("simpleRequest: not able to get next object size");
            std::cout << "simpleRequest: not able to get next object size" << std::endl;
            if (retries < MAX_RETRIES) {
                retries++;
                continue;
            } else {
                return onErr;
            }
        }
        void* memory = malloc(objectSize);
        if (memory == nullptr) {
            myLogger->error(std::string("FATAL ERROR: not able to allocate memory with size=") +
                            std::to_string(objectSize));
            std::cout << "FATAL ERROR: not able to allocate memory" << std::endl;
            exit(-1);
        }
        ReturnType finalResult;
        {
            Handle<ResponseType> result = temp.getNextObject<ResponseType>(memory, success, errMsg);
            if (!success) {
                myLogger->error(errMsg);
                myLogger->error("simpleRequest: not able to get next object over the wire.\n");
                // JiaNote: we need free memory here!!!
                free(memory);
                if (retries < MAX_RETRIES) {
                    retries++;
                    continue;
                } else {
                    return onErr;
                }
            }

            finalResult = processResponse(result);
        }

        free(memory);
        return finalResult;
    }
    return onErr;
}
}
#endif
