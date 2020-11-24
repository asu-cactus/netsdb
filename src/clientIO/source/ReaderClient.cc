#ifndef READER_CLIENT_CC
#define READER_CLIENT_CC

#include "PDBClient.h"
#include "ReaderClient.h"

#include <queue>
#include <string>
#include <mutex>
#include <thread>


// ReaderClient::ReaderClient(int port, std::string managerIp,
// pdb::PDBLoggerPtr clientLogger){
//     this->port = port;
//     this->managerIp = managerIp;
//     this->clientLogger = clientLogger;
//     this->inMutex = new std::mutex();
//     this->countMutex = new std::mutex();
//     this->currRowCount = 0;
//     this->maxFlag = false;
// }

// ReaderClient::~ReaderClient(){
//     delete this->countMutex;
//     delete this->inMutex;
// }

// template <typename T>
// void ReaderClient::load(int numOfThreads, std::queue<std::ifstream *> &inFiles,
// std::string dbName, std::string setName, int blockSizeInMB,
// std::string objectPath, long maxRowCount){
//     std::vector<std::thread> threadVec;
//     for(int i = 0; i < numOfThreads; i++){
//         threadVec.push_back(std::thread(
//             readerRoutine<T>, std::cref(inFiles), dbName, setName,
//             blockSizeInMB, this->port, this->managerIp, this->clientLogger,
//             objectPath, this->inMutex, this->countMutex, maxRowCount,
//             std::cref(this->currRowCount), std::cref(this->maxFlag)
//             ));
//         cout << "READER CLIENT: Thread " << i << " started."
//             << std::endl;
//     }
//     for (int i = 0; i < numOfThreads; i++){
//         threadVec.at(i).join();
//         std::cout << "READER CLIENT: Thread " << i << " joined."
//             << std::endl;
//     }
// }

#endif