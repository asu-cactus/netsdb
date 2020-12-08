#ifndef READER_CLIENT_H
#define READER_CLIENT_H

#include "PDBClient.h"

#include <queue>
#include <string>
#include <mutex>
#include <thread>
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD


=======
#include <functional>

template <typename T>
>>>>>>> Initial code for threded multi-client read operation.
=======


>>>>>>> Reader client for multi thread loading
=======


>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
class ReaderClient {
private:
    std::mutex *inMutex;
    std::mutex *countMutex;
    long currRowCount;
    bool maxFlag;
protected:
    int port;
    std::string managerIp;
    pdb::PDBLoggerPtr clientLogger;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
  
public:
    ReaderClient(int port, std::string managerIp,
    pdb::PDBLoggerPtr clientLogger){
        this->port = port;
        this->managerIp = managerIp;
        this->clientLogger = clientLogger;
=======
    std::string objectToRegister;
    long maxRowCount = LONG_MAX;
=======
>>>>>>> Reader client for multi thread loading
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
  
public:
    ReaderClient(int port, std::string managerIp,
    pdb::PDBLoggerPtr clientLogger){
        this->port = port;
        this->managerIp = managerIp;
        this->clientLogger = clientLogger;
<<<<<<< HEAD
<<<<<<< HEAD
        this->maxRowCount = total;
        this->objectToRegister = objectPath;
>>>>>>> Initial code for threded multi-client read operation.
=======
>>>>>>> Reader client for multi thread loading
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
        this->inMutex = new std::mutex();
        this->countMutex = new std::mutex();
        this->currRowCount = 0;
        this->maxFlag = false;
    }

    ~ReaderClient(){
        delete this->countMutex;
        delete this->inMutex;
    }

    /**
     * Method to spawn the requested number of threads to load
     * a set of files.
     * @param numOfThreads Number of concurrent files to process
     * @param inFiles Referece to the queue of input streams
     * @param dbName Database name
     * @param setName Name of the data-set
     * @param blockSizeInMB Block of data to be sent for each
     * communication to the Dispatcher.
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
     * @param objectPath PDB object to register
     * @param maxRowCount Max row count for the set
     */
    template <typename T>
    void load(int numOfThreads, std::queue<std::ifstream *> &inFiles,
    std::string dbName, std::string setName, int blockSizeInMB,
    std::string objectPath, long maxRowCount){
        std::vector<std::thread> threadVec;
        for(int i = 0; i < numOfThreads; i++){
            threadVec.push_back(std::thread(
                readerRoutine<T>, std::cref(inFiles), dbName, setName,
                blockSizeInMB, this->port, this->managerIp, this->clientLogger,
                objectPath, this->inMutex, this->countMutex, maxRowCount,
                std::cref(this->currRowCount), std::cref(this->maxFlag)
                ));
<<<<<<< HEAD
=======
=======
     * @param objectPath PDB object to register
     * @param maxRowCount Max row count for the set
>>>>>>> Reader client for multi thread loading
     */
    template <typename T>
    void load(int numOfThreads, std::queue<std::ifstream *> &inFiles,
    std::string dbName, std::string setName, int blockSizeInMB,
    std::string objectPath, long maxRowCount){
        std::vector<std::thread> threadVec;
        for(int i = 0; i < numOfThreads; i++){
            threadVec.push_back(std::thread(
                readerRoutine<T>, std::cref(inFiles), dbName, setName,
                blockSizeInMB, this->port, this->managerIp, this->clientLogger,
                objectPath, this->inMutex, this->countMutex, maxRowCount,
                std::cref(this->currRowCount), std::cref(this->maxFlag)
<<<<<<< HEAD
                std::ref(&(this->rowParser))));
>>>>>>> Initial code for threded multi-client read operation.
=======
                ));
>>>>>>> Reader client for multi thread loading
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
            cout << "READER CLIENT: Thread " << i << " started."
                << std::endl;
        }
        for (int i = 0; i < numOfThreads; i++){
            threadVec.at(i).join();
            std::cout << "READER CLIENT: Thread " << i << " joined."
                << std::endl;
        }
    }

protected:
    /**
     * Method to parse each row from the input file.
     * 
     * @param line String input for the file reader for each row.
     * @param currCount Index to which current row is sent.
     */
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    template <typename P>
    static pdb::Handle<P> rowParser(std::string line,
    int currCount){ return pdb::makeObject<P>(line); }
=======
    pdb::Handle<T> rowParser(std::string line, int currCount);
>>>>>>> Initial code for threded multi-client read operation.
=======
    template <typename P>
    static pdb::Handle<P> rowParser(std::string line,
    int currCount){ return pdb::makeObject<P>(line); }
>>>>>>> Reader client for multi thread loading
=======
    template <typename P>
    static pdb::Handle<P> rowParser(std::string line,
    int currCount){ return pdb::makeObject<P>(line); }
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5

private:
    /**
     * Routine to execute for each input file stream.
     * 
     * This method will be called inside every thread,
     * while each thread reading a part file at a time.
     * 
     * @param inFiles Referece to the queue of input streams
     * @param dbName Database name
     * @param setName Name of the data-set
     * @param blockSizeInMB Block of data to be sent for each
     * @param port Port of the Server/Manager process
     * @param managerIp IP address of the Manager/Server/Driver
     * @param clientLogger PDB client logger pointer
     * @param objectToRegister PDB Object for the data to be loaded
     * @param inMutex Mutex to gaurd the File streams queue
     * @param countMutex Mutex to gaurd the count of rows loaded
     * @param maxRowCount Maximum rows to load
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
     * @param flag Flag to check if the max row count is reached
     * communication to the Dispatcher.
     */
    template <typename R>
=======
     * @param maxFlag Flag to check if the max row count is reached
     * @param parseRow Function to parse each line into row object
     * communication to the Dispatcher.
     */
>>>>>>> Initial code for threded multi-client read operation.
=======
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
     * @param flag Flag to check if the max row count is reached
     * communication to the Dispatcher.
     */
    template <typename R>
<<<<<<< HEAD
>>>>>>> Reader client for multi thread loading
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
    static void readerRoutine(std::queue<std::ifstream *> const &inFiles,
    std::string dbName, std::string setName, int blockSizeInMB, int port, 
    std::string managerIp, pdb::PDBLoggerPtr clientLogger,
    std::string objectToRegister, std::mutex *inMutex, std::mutex *countMutex,
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    int maxRowCount, int const &rowCount, bool const &flag){
=======
    int maxRowCount, int const &rowCount, bool const &flag,
    std::function<pdb::Handle<T>(std::string, int)> &parserFunc){
>>>>>>> Initial code for threded multi-client read operation.
=======
    int maxRowCount, int const &rowCount, bool const &flag){
>>>>>>> Reader client for multi thread loading
=======
    int maxRowCount, int const &rowCount, bool const &flag){
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
        int &count = const_cast<int &>(rowCount);
        bool &stopFlag = const_cast<bool &>(flag);
        std::queue<std::ifstream *> &inFileQ = 
            const_cast<std::queue<std::ifstream *> &>(inFiles);
        std :: string line;
        bool rollback = false;
        bool end = false;
        string errMsg = "Error occurred in the reading client threads.";
        pdb::PDBClient pdbClient(port, managerIp, clientLogger, false, true);
        std::cout << "Registering Object " << objectToRegister << std::endl; 
        pdbClient.registerType(objectToRegister, errMsg);
        // Keep reading the files until all the files are read
        while(!inFiles.empty()){
            cout << "Waiting for file mutex to access." << std::endl;
            inMutex->lock();
            std::ifstream *currFile = inFileQ.front();
            inFileQ.pop();
            inMutex->unlock();
            std::cout << "File pointer poped from the queue." << std::endl;
            rollback = false;
            end = false;
            pdb::makeObjectAllocatorBlock(
                (size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
            pdb::Handle<pdb::Vector<pdb::Handle<R>>> storeMe = 
                pdb::makeObject<pdb::Vector<pdb::Handle<R>>>();
=======
            pdb::Handle<pdb::Vector<pdb::Handle<T>>> storeMe = 
                pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
>>>>>>> Initial code for threded multi-client read operation.
=======
            pdb::Handle<pdb::Vector<pdb::Handle<R>>> storeMe = 
                pdb::makeObject<pdb::Vector<pdb::Handle<R>>>();
>>>>>>> Reader client for multi thread loading
=======
            pdb::Handle<pdb::Vector<pdb::Handle<R>>> storeMe = 
                pdb::makeObject<pdb::Vector<pdb::Handle<R>>>();
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
            while (!end) {
                // Roll back one line, hence skip reading new line from stream.
                // And load the previously fetched line.
                if (!rollback) {
                    if(!std::getline(*currFile, line)){
                        end = true;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
                        if (!pdbClient.sendData<R>(
=======
                        if (!pdbClient.sendData<T>(
>>>>>>> Initial code for threded multi-client read operation.
=======
                        if (!pdbClient.sendData<R>(
>>>>>>> Reader client for multi thread loading
=======
                        if (!pdbClient.sendData<R>(
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
                                std::pair<std::string,
                                std::string>(setName, dbName),
                                storeMe, errMsg)){
                            std::cout << "Failed to send data to dispatcher server"
                                << std::endl;
                            return;
                        }
                        pdbClient.flushData (errMsg);
                        std::cout << "Dispatched " << storeMe->size()
                            << setName << std::endl;
                        break;
                    }
                }
                // Keep loading the local [[pdb::Handle]] with each line.
                rollback = false; 
                try {
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
                    pdb::Handle<R> row = rowParser<R>(line, count);
                    storeMe->push_back(row);
                }
                catch (pdb::NotEnoughSpace &n) {
                    if (! pdbClient.sendData<R>(
<<<<<<< HEAD
=======
                    pdb::Handle<T> row = parserFunc(line, count);
                    storeMe->push_back(row);
                }
                catch (pdb::NotEnoughSpace &n) {
                    if (! pdbClient.sendData<T>(
>>>>>>> Initial code for threded multi-client read operation.
=======
                    pdb::Handle<R> row = rowParser<R>(line, count);
                    storeMe->push_back(row);
                }
                catch (pdb::NotEnoughSpace &n) {
                    if (! pdbClient.sendData<R>(
>>>>>>> Reader client for multi thread loading
=======
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
                        std::pair<std::string, std::string>(setName, dbName),
                        storeMe, errMsg
                        )) {
                        std::cout << "Failed to send data to dispatcher server"
                            << std::endl;
                        return;
                    }
                    std::cout << "Dispatched " << storeMe->size()
                        << setName << std::endl;
                    countMutex->lock();
                    count = count + storeMe->size();
                    countMutex->unlock();
                    std::cout << "sent " << count
                        << " objects in total" << std::endl;
                    // end the loading if the count 
                    if (count >= maxRowCount) { 
                        end = true;
                        stopFlag = true;
                    }
                    // Roll back one line if current row loading fails.
                    rollback = true; 
                    pdb::makeObjectAllocatorBlock(
                        (size_t)blockSizeInMB * (size_t)1024 * (size_t)1024,
                        true);
                    storeMe =
                        pdb::makeObject<
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
                            pdb::Vector<pdb::Handle<R>>
=======
                            pdb::Vector<pdb::Handle<T>>
>>>>>>> Initial code for threded multi-client read operation.
=======
                            pdb::Vector<pdb::Handle<R>>
>>>>>>> Reader client for multi thread loading
=======
                            pdb::Vector<pdb::Handle<R>>
>>>>>>> ff93e0b3e5d3811b9eab951fc6e26a8146bf56d5
                        >();
                }
            }
            pdbClient.flushData(errMsg);
            // Stop reading more File streams, if max row count has been reached.
            if(stopFlag){
                delete currFile;
                std::cout << "Maximun row count reached." << std::endl;
                std::cout << "Waiting for file mutex to clear all file pointers."
                    << std::endl;
                inMutex->lock();
                while (!inFiles.empty()){
                    std::ifstream *fp = inFileQ.front();
                    delete fp;
                    inFileQ.pop();
                }
                cout << "Cleared all the allocated file pointers." << std::endl;
                inMutex->unlock();
                break;
            } 
            delete currFile;
        }
    }
};

#endif