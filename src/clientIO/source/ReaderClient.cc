#ifndef READER_CLIENT_CC
#define READER_CLIENT_CC

#include "PDBClient.h"
#include "ReaderClient.h"

#include <queue>
#include <string>
#include <mutex>
#include <thread>


<<<<<<< HEAD
<<<<<<< HEAD
// ReaderClient::ReaderClient(int port, std::string managerIp,
// pdb::PDBLoggerPtr clientLogger){
//     this->port = port;
//     this->managerIp = managerIp;
//     this->clientLogger = clientLogger;
=======
// template <typename T>
// pdb::ReaderClient<T>::ReaderClient(int port, std::string managerIp,
//     pdb::PDBLoggerPtr clientLogger, long total, std::string objectPath){
//     this->port = port;
//     this->managerIp = managerIp;
//     this->clientLogger = clientLogger;
//     this->maxRowCount = total;
//     this->objectToRegister = objectPath;
>>>>>>> Initial code for threded multi-client read operation.
=======
// ReaderClient::ReaderClient(int port, std::string managerIp,
// pdb::PDBLoggerPtr clientLogger){
//     this->port = port;
//     this->managerIp = managerIp;
//     this->clientLogger = clientLogger;
>>>>>>> Reader client for multi thread loading
//     this->inMutex = new std::mutex();
//     this->countMutex = new std::mutex();
//     this->currRowCount = 0;
//     this->maxFlag = false;
// }

<<<<<<< HEAD
<<<<<<< HEAD
// ReaderClient::~ReaderClient(){
=======
// template <typename T>
// pdb::ReaderClient<T>::~ReaderClient(){
>>>>>>> Initial code for threded multi-client read operation.
=======
// ReaderClient::~ReaderClient(){
>>>>>>> Reader client for multi thread loading
//     delete this->countMutex;
//     delete this->inMutex;
// }

// template <typename T>
<<<<<<< HEAD
<<<<<<< HEAD
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
=======
// void pdb::ReaderClient<T>::exec(int numOfThreads,
// std::queue<std::ifstream *> &inFiles, std::string dbName, std::string setName,
// int blockSizeInMB){
//     std::vector<std::thread> threadVec;
//     for(int i = 0; i < numOfThreads; i++){
//         threadVec.push_back(std::thread(readerRoutine, std::cref(inFiles), 
//             dbName, setName, blockSizeInMB, this->port, this->managerIp,
//             this->clientLogger, this->objectToRegister ,this->inMutex,
//             this->countMutex, this->maxRowCount,
//             std::cref(this->currRowCount), std::cref(this->maxFlag)));
>>>>>>> Initial code for threded multi-client read operation.
=======
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
>>>>>>> Reader client for multi thread loading
//         cout << "READER CLIENT: Thread " << i << " started."
//             << std::endl;
//     }
//     for (int i = 0; i < numOfThreads; i++){
//         threadVec.at(i).join();
//         std::cout << "READER CLIENT: Thread " << i << " joined."
//             << std::endl;
//     }
// }

<<<<<<< HEAD
<<<<<<< HEAD
=======
// template <typename T>
// void pdb::ReaderClient<T>::readerRoutine(std::queue<std::ifstream *> const &inFiles,
// std::string dbName, std::string setName, int blockSizeInMB, int port, 
// std::string managerIp, pdb::PDBLoggerPtr clientLogger,
// std::string objectToRegister, std::mutex *inMutex, std::mutex *countMutex,
// int maxRowCount, int const &rowCount, bool const &flag){
//     int &count = const_cast<int &>(rowCount);
//     bool &stopFlag = const_cast<bool &>(flag);
//     std::queue<std::ifstream *> &inFileQ = 
//         const_cast<std::queue<std::ifstream *> &>(inFiles);
//     std :: string line;
//     bool rollback = false;
//     bool end = false;
//     string errMsg = "Error occurred in the reading client threads.";
//     pdb::PDBClient pdbClient(port, managerIp, clientLogger, false, true);
//     std::cout << "Registering Object " << objectToRegister << std::endl; 
//     pdbClient.registerType(objectToRegister, errMsg);
//     // Keep reading the files until all the files are read
//     while(!inFiles.empty()){
//         inMutex->lock();
//         std::ifstream *currFile = inFileQ.front();
//         inFileQ.pop();
//         inMutex->unlock();
//         rollback = false;
//         end = false;
//         pdb::makeObjectAllocatorBlock(
//             (size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
//         pdb::Handle<pdb::Vector<pdb::Handle<T>>> storeMe = 
//             pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
//         while (!end) {
//             // Roll back one line, hence skip reading new line from stream.
//             // And load the previously fetched line.
//             if (!rollback) {
//                 if(!std::getline(*currFile, line)){
//                     end = true;
//                     if (!pdbClient.sendData<T>(
//                             std::pair<std::string,
//                             std::string>(setName, dbName),
//                             storeMe, errMsg)){
//                         std::cout << "Failed to send data to dispatcher server"
//                             << std::endl;
//                         return;
//                     }
//                     pdbClient.flushData (errMsg);
//                     std::cout << "Dispatched " << storeMe->size()
//                         << setName << std::endl;
//                     break;
//                 }
//             }
//             // Keep loading the local [[pdb::Handle]] with each line.
//             rollback = false; 
//             try {
//                 pdb::Handle<T> row = rowParser(line, count);
//                 storeMe->push_back(row);
//             }
//             catch (pdb::NotEnoughSpace &n) {
//                 if (! pdbClient.sendData<T>(
//                     std::pair<std::string, std::string>(setName, dbName),
//                     storeMe, errMsg
//                     )) {
//                     std::cout << "Failed to send data to dispatcher server"
//                         << std::endl;
//                     return;
//                 }
//                 std::cout << "Dispatched " << storeMe->size()
//                     << setName << std::endl;
//                 countMutex->lock();
//                 count = count + storeMe->size();
//                 countMutex->unlock();
//                 std::cout << "sent " << count
//                     << " objects in total" << std::endl;
//                 // end the loading if the count 
//                 if (count >= maxRowCount) { 
//                     end = true;
//                     stopFlag = true;
//                 }
//                 // Roll back one line if current row loading fails.
//                 rollback = true; 
//                 pdb::makeObjectAllocatorBlock(
//                     (size_t)blockSizeInMB * (size_t)1024 * (size_t)1024,
//                     true);
//                 storeMe =
//                     pdb::makeObject<
//                         pdb::Vector<pdb::Handle<T>>
//                     >();
//             }
//         }
//         pdbClient.flushData(errMsg);
//         // Stop reading more File streams, if max row count has been reached.
//         if(stopFlag) break;
//         delete currFile;
//     }
// }

>>>>>>> Initial code for threded multi-client read operation.
=======
>>>>>>> Reader client for multi thread loading
#endif