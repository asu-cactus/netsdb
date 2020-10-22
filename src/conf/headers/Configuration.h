#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <assert.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include "DataTypes.h"

#include "LogLevel.h"


using namespace std;

#ifndef DEFAULT_PAGE_SIZE
#define DEFAULT_PAGE_SIZE ((size_t)(128) * (size_t)(1024) * (size_t)(1024))
#endif

#ifndef DEFAULT_MAX_PAGE_SIZE
#define DEFAULT_MAX_PAGE_SIZE ((size_t)(512) * (size_t)(1024) * (size_t)(1024))
#endif

#ifndef DEFAULT_NET_PAGE_SIZE
#define DEFAULT_NET_PAGE_SIZE                                                       \
    DEFAULT_PAGE_SIZE - (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) + \
                         sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t))
#endif

#ifndef DEFAULT_SHUFFLE_PAGE_SIZE
#define DEFAULT_SHUFFLE_PAGE_SIZE size_t(256) * size_t(1024) * size_t(1024)
#endif

#ifndef DEFAULT_BROADCAST_PAGE_SIZE
#define DEFAULT_BROADCAST_PAGE_SIZE size_t(128) * size_t(1024) * size_t(1024)
#endif


#ifndef DEFAULT_MAX_CONNECTIONS
#define DEFAULT_MAX_CONNECTIONS 200
#endif

#ifndef DEFAULT_SHAREDMEM_SIZE
#define DEFAULT_SHAREDMEM_SIZE ((size_t)14 * (size_t)1024 * (size_t)1024 * (size_t)1024)
#endif

#ifndef DEFAULT_NUM_THREADS
#define DEFAULT_NUM_THREADS 2
#endif

#ifndef DEFAULT_BATCH_SIZE
#define DEFAULT_BATCH_SIZE 100
#endif


#ifndef DEFAULT_HASH_PAGE_SIZE
#define DEFAULT_HASH_PAGE_SIZE ((size_t)(512) * (size_t)(1024) * (size_t)(1024))
#endif

// unit: KB
#ifndef DEFAULT_MEM_SIZE
#define DEFAULT_MEM_SIZE ((size_t)(32) * (size_t)(1024) * (size_t)(1024))
#endif

#ifndef DEFAULT_NUM_CORES
#define DEFAULT_NUM_CORES 4 
#endif

// create a smart pointer for Configuration objects
class Configuration;
typedef shared_ptr<Configuration> ConfigurationPtr;

class Configuration {
private:
    NodeID nodeId;
    string serverName;
    string serverAddress;
    bool isMasterCatalogServer;
    bool usePangea;
    int port;
    int maxConnections;
    string ipcFile;
    string logFile;
    size_t pageSize;
    size_t shufflePageSize;
    size_t broadcastPageSize;
    size_t maxPageSize;
    bool useUnixDomainSock;
    size_t shmSize;
    bool logEnabled;
    string dataDirs;
    string metaDir;
    string metaTempDir;
    string dataTempDirs;
    unsigned int numThreads;
    string backEndIpcFile;
    int batchSize;
    size_t hashPageSize;
    bool isMaster;
    string masterNodeHostName;
    int masterNodePort;
    string queryPlannerPlace;
    LogLevel logLevel;
    string rootDir;
    string selfLearningDB;

public:
    Configuration() {
        // set default values.
        this->nodeId = 0;
        this->logLevel = LogLevel::ERROR;
        serverName = "testServer";
        serverAddress = "localhost";
        isMasterCatalogServer = false;
        usePangea = true;
        port = 8108;
        maxConnections = DEFAULT_MAX_CONNECTIONS;
        logFile = "serverLog";
        maxPageSize = DEFAULT_MAX_PAGE_SIZE;
        pageSize = DEFAULT_PAGE_SIZE;
        assert(pageSize <= maxPageSize);
        shufflePageSize = DEFAULT_SHUFFLE_PAGE_SIZE;
        assert(shufflePageSize <= maxPageSize);
        broadcastPageSize = DEFAULT_BROADCAST_PAGE_SIZE;
        assert(broadcastPageSize <= maxPageSize);
        useUnixDomainSock = false;
        shmSize = DEFAULT_SHAREDMEM_SIZE;
        logEnabled = false;
        numThreads = DEFAULT_NUM_THREADS;
        ipcFile = "/tmp/ipcFile";
        backEndIpcFile = "/tmp/backEndIpcFile";
        batchSize = DEFAULT_BATCH_SIZE;
        isMaster = false;
        hashPageSize = DEFAULT_HASH_PAGE_SIZE;
        initDirs();
        selfLearningDB = "selfLearningDB";
    }

    void initDirs() {
        rootDir = std::string("pdbRoot_") + serverAddress + std::string("_") + std::to_string(port);
        // temporarily added for unit tests
        this->createDir(rootDir);
        // Example: 
        dataDirs = rootDir + std::string("/data");
        dataTempDirs = rootDir + std::string("/tmp");
        //dataDirs = rootDir + std::string("/data");
        metaDir = rootDir + std::string("/meta");
        metaTempDir = rootDir + std::string("/metaTmp");
        //dataTempDirs = rootDir + std::string("/tmp");

    }


    NodeID getNodeID() const {
        return nodeId;
    }

    string getServerName() const {
        return serverName;
    }

    string getIpcFile() const {
        return ipcFile;
    }

    string getLogFile() const {
        return logFile;
    }

    int getMaxConnections() const {
        return maxConnections;
    }

    size_t getPageSize() const {
        return pageSize;
    }

    size_t getNetPageSize() const {
        return pageSize - (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
                           sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t));
    }


    size_t getMaxPageSize() const {
        return maxPageSize;
    }

    size_t getShufflePageSize() const {
        return shufflePageSize;
    }

    size_t getNetShufflePageSize() const {
        return shufflePageSize - (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
                                  sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t));
    }

    size_t getBroadcastPageSize() const {
        return broadcastPageSize;
    }

    size_t getNetBroadcastPageSize() const {
        return broadcastPageSize - (sizeof(NodeID) + sizeof(DatabaseID) + sizeof(UserTypeID) +
                                    sizeof(SetID) + sizeof(PageID) + sizeof(int) + sizeof(size_t));
    }


    size_t getHashPageSize() const {
        return hashPageSize;
    }

    int getPort() const {
        return port;
    }

    size_t getShmSize() const {
        return shmSize;
    }

    bool isLogEnabled() const {
        return logEnabled;
    }

    bool isUseUnixDomainSock() const {
        return useUnixDomainSock;
    }

    string getDataDirs() const {
        return dataDirs;
    }

    string getMetaDir() const {
        return metaDir;
    }

    string getMetaTempDir() const {
        return metaTempDir;
    }

    string getDataTempDirs() const {
        return dataTempDirs;
    }

    unsigned int getNumThreads() const {
        return numThreads;
    }

    string getBackEndIpcFile() const {
        return backEndIpcFile;
    }

    void setNodeId(NodeID nodeId) {
        this->nodeId = nodeId;
    }

    void setServerName(string serverName) {
        this->serverName = serverName;
    }

    void setIpcFile(string ipcFile) {
        this->ipcFile = ipcFile;
    }

    void setLogFile(string logFile) {
        this->logFile = logFile;
    }

    void setMaxConnections(int maxConnections) {
        this->maxConnections = maxConnections;
    }

    void setPageSize(size_t pageSize) {
        assert(pageSize <= maxPageSize);
        this->pageSize = pageSize;
    }

    void setMaxPageSize(size_t maxPageSize) {
        assert(pageSize <= maxPageSize);
        assert(shufflePageSize <= maxPageSize);
        assert(broadcastPageSize <= maxPageSize);
        this->maxPageSize = maxPageSize;
    }

    void setShufflePageSize(size_t shufflePageSize) {
        assert(shufflePageSize < maxPageSize);
        this->shufflePageSize = shufflePageSize;
    }

    void setHashPageSize(size_t hashPageSize) {
        this->hashPageSize = hashPageSize;
    }

    void setBroadcastPageSize(size_t broadcastPageSize) {
        assert(broadcastPageSize <= maxPageSize);
        this->broadcastPageSize = broadcastPageSize;
    }


    void setPort(int port) {
        this->port = port;
    }

    void setShmSize(size_t shmSize) {
        this->shmSize = shmSize;
    }

    void setUseUnixDomainSock(bool useUnixDomainSock) {
        this->useUnixDomainSock = useUnixDomainSock;
    }

    void setLogEnabled(bool logEnabled) {
        this->logEnabled = logEnabled;
    }

    void setDataDirs(string dataDirs) {
        this->dataDirs = dataDirs;
    }

    void setMetaDir(string metaDir) {
        this->metaDir = metaDir;
    }

    void setMetaTempDir(string tempDir) {
        this->metaTempDir = tempDir;
    }

    void setDataTempDirs(string tempDirs) {
        this->dataTempDirs = tempDirs;
    }

    void setNumThreads(unsigned int numThreads) {
        this->numThreads = numThreads;
    }

    void setBackEndIpcFile(string backEndIpcFile) {
        this->backEndIpcFile = backEndIpcFile;
    }

    void createDir(string path) {
        struct stat st = {0};
        if (stat(path.c_str(), &st) == -1) {
            mkdir(path.c_str(), 0777);
        }
    }

    bool getIsMaster() const {
        return isMaster;
    }

    void setIsMaster(bool isMaster) {
        this->isMaster = isMaster;
    }

    string getMasterNodeHostName() const {
        return masterNodeHostName;
    }

    void setMasterNodeHostName(string masterNodeHostName) {
        this->masterNodeHostName = masterNodeHostName;
    }

    int getMasterNodePort() const {
        return masterNodePort;
    }

    void setMasterNodePort(int masterNodePort) {
        this->masterNodePort = masterNodePort;
    }

    const string getQueryPlannerPlace() const {
        return queryPlannerPlace;
    }

    void setQueryPlannerPlace(const string queryPlannerPlace) {
        this->queryPlannerPlace = queryPlannerPlace;
    }

    LogLevel getLogLevel() const {
        return logLevel;
    }

    void setLogLevel(LogLevel logLevel) {
        this->logLevel = logLevel;
    }

    void setMasterCatalogServer(bool isMasterCatalogServer) {
        this->isMasterCatalogServer = isMasterCatalogServer;
    }

    bool getMasterCatalogServer() {
        return this->isMasterCatalogServer;
    }

    void setServerAddress(string serverAddress) {
        this->serverAddress = serverAddress;
    }

    string getServerAddress() {
        return this->serverAddress;
    }

    void setUsePangea(bool usePangea) {
        this->usePangea = usePangea;
    }

    bool getUsePangea() {
        return this->usePangea;
    }

    int getBatchSize() {
        return this->batchSize;
    }

    void setBatchSize(int batchSize) {
        this->batchSize = batchSize;
    }

    std::string getSelfLearningDB() {
        return this->selfLearningDB;
    }

    void setSelfLearningDB(std::string selfLearningDB) {
        this->selfLearningDB = selfLearningDB;
    }


    void printOut() {
        cout << "nodeID: " << nodeId << endl;
        cout << "serverName: " << serverName << endl;
        cout << "serverAddress: " << serverAddress << endl;
        cout << "isMasterCatalogServer: " << isMasterCatalogServer << endl;
        cout << "usePangea: " << usePangea << endl;
        cout << "port: " << port << endl;
        cout << "maxConnections: " << maxConnections << endl;
        cout << "ipcFile: " << ipcFile << endl;
        cout << "logFile: " << logFile << endl;
        cout << "pageSize: " << pageSize << endl;
        cout << "maxPageSize: " << maxPageSize << endl;
        cout << "shufflePageSize: " << shufflePageSize << endl;
        cout << "broadcastPageSize: " << broadcastPageSize << endl;
        cout << "hashPageSize: " << hashPageSize << endl;
        cout << "useUnixDomainSock: " << useUnixDomainSock << endl;
        cout << "shmSize: " << shmSize << endl;
        cout << "dataDirs: " << dataDirs << endl;
        cout << "metaDir: " << metaDir << endl;
        cout << "metaTempDir: " << metaTempDir << endl;
        cout << "dataTempDirs: " << dataTempDirs << endl;
        cout << "numThreads: " << numThreads << endl;
        cout << "backEndIpcFile: " << backEndIpcFile << endl;
        cout << "isMaster: " << isMaster << endl;
        cout << "masterNodeHostName: " << masterNodeHostName << endl;
        cout << "masterNodePort: " << masterNodePort << endl;
        cout << "logEnabled: " << logEnabled << endl;
        cout << "batchSize: " << batchSize << endl;
        cout << "selfLearningDB: " << selfLearningDB << endl;
    }
};

#endif /* CONFIGURATION_H */
