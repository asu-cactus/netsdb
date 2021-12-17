
#ifndef PANGEA_STORAGE_SERVER_H
#define PANGEA_STORAGE_SERVER_H



#include "ServerFunctionality.h"
#include "PDBServer.h"
#include "Record.h"
#include "PDBVector.h"
#include "Configuration.h"
#include "DataTypes.h"
#include "PDBLogger.h"
#include "DefaultDatabase.h"
#include "SharedMem.h"
#include "TempSet.h"
#include "PDBWork.h"
#include <vector>
#include <string>
#include <map>
#include <boost/filesystem.hpp>
#include <pthread.h>
#include <memory>


namespace pdb {


class PangeaStorageServer;
typedef std::shared_ptr<PangeaStorageServer> PangeaStorageServerPtr;

//this class encapsulates the PangeaStorageServer functionality
//It should be installed on each Worker node, and managed by DistributedStorageManager and Dispatcher.
//It includes a PageCache as a buffer pool, and a group of PartitionedFile instances to serve as user-level file system.
//The organization of user data has two levels: Database and UserSet, and one database can have multiple user sets.

//As a local server, it handles user requests for following storage services:
//-- StorageCleanup: to force a flush of all pages
//-- StorageAddDatabase: to add a database
//-- StorageAddSet: to add a user set
//-- StorageAddTempSet: to add a temp set, that is transient and not need registration with catalog
//-- StorageRemoveDatabase: to remove a database
//-- StorageRemoveUserSet: to remove a user set
//-- StorageClearSet: to remove data from a set
//-- StorageRemoveTempSet: to remove a temp set
//-- StorageAddObjectInLoop: to add large objects in loop, and add one large object each time
//-- StorageAddData: to add a Vector<Object> to a set
//-- StorageGetData: to return data from a set
//-- StorageCollectStats: to collect stats for this local storage
//-- StoragePinPage: to pin a page in one set
//-- StoragePinBytes: to pin bytes of specified length in one set
//-- StorageUnpinPage: to unpin a page from one set
//-- StorageGetSetPages: to trigger a parallel scan over a set





class PangeaStorageServer : public ServerFunctionality {

public:
    // creates a storage server, putting all data in the indicated directory.
    // pages can be dynamically allocated from the specified shared memory pool.
    PangeaStorageServer(SharedMemPtr shm,
                        PDBWorkerQueuePtr workers,
                        PDBLoggerPtr logger,
                        ConfigurationPtr conf,
                        bool standalone = true);


    /*****************To Comply with Chris' Interfaces***************************/


    // takes all of the currently buffered records for the given database/set pair,
    // creates at most one page of data with them, and then writes that page to storage
    void writeBackRecords(pair<std::string, std::string> databaseAndSet,
                          bool flushOrNot = true,
                          bool directPutOrNot = false);

    // this allocates a new page at the end of the indicated database/set combo
    PDBPagePtr getNewPage(pair<std::string, std::string> databaseAndSet);

    // returns a set object referencing the given database/set pair
    SetPtr getSet(std::pair<std::string, std::string> databaseAndSet);


    // from the ServerFunctionality interface... registers the StorageServer's handlers
    void registerHandlers(PDBServer& forMe) override;

    // stores a record---we'll keep buffering records until we get enough of them that
    // we can put them together into a page.  The return value is the total size of
    // all of the records that we are buffering for this database and set
    size_t bufferRecord(pair<std::string, std::string> databaseAndSet,
                        Record<Vector<Handle<Object>>>* addMe);

    // destructor
    ~PangeaStorageServer();


    /****************Pangea Interfaces******************************************/

    /**
     * Returns server name
     */
    string getServerName();

    /**
     * Returns nodeId
     */
    NodeID getNodeId();

    /**
     * Returns the ipc path to backEnd
     */
    string getPathToBackEndServer();

    /**
     * Returns logger
     */
    PDBLoggerPtr getLogger();

    /**
     * Returns configuration
     */
    ConfigurationPtr getConf();

    /**
     * Returns shared memory handle, for caching
     */
    SharedMemPtr getSharedMem();

    /**
     * Returns cache
     */
    PageCachePtr getCache();

    /**
     * returns the flush buffer
     */
    PageCircularBufferPtr getFlushBuffer();

    /**
     * Add a new and empty database
     */
    bool addDatabase(std::string dbName, DatabaseID dbId);

    /**
     * Add a new and empty database using only name
     */
    bool addDatabase(std::string dbName);


    /**
     * Remove an existing database
     */
    bool removeDatabase(std::string dbName);


    /**
     * Add a new and empty type
     */
    bool addType(std::string typeName, UserTypeID typeId);

    /**
     * Remove a type from a database, and also remove all sets in the database having that type
     */
    bool removeTypeFromDatabase(std::string dbName, std::string typeName);

    /**
     * Remove a type from the typeName to typeId mapping
     */
    bool removeType(std::string typeName);

    /**
     * Add a new and empty set
     */
    bool addSet(std::string dbName,
                std::string typeName,
                std::string setName,
                SetID setId,
                size_t pageSize = DEFAULT_PAGE_SIZE,
                size_t desiredSize = 1,
                bool isMRU = true,
                bool isTransient = true,
		bool isSharedTensorBlockSet = false);


    /**
     * Add a new and empty set using only name
     */
    bool addSet(std::string dbName,
                std::string typeName,
                std::string setName,
                size_t pageSize = DEFAULT_PAGE_SIZE,
                size_t desiredSize = 1,
                bool isMRU = true,
                bool isTransient = true,
		bool isSharedTensorBlockSet = false);

    /**
     * Add a set using only database name and set name
     */
    bool addSet(std::string dbName, 
                std::string setName, 
                size_t pageSize = DEFAULT_PAGE_SIZE,
                size_t desiredSize = 1,
                bool isMRU = true,
                bool isTransient = true,
		bool isSharedTensorBlockSet = false);


    /**
     * Remove an existing set
     */
    bool removeSet(std::string dbName, std::string typeName, std::string setName);

    /**
     * Remove an existing set
     */
    bool removeSet(std::string dbName, std::string setName);


    /**
     * Returns a specified database.
     * If database doesn't exist, returns nullptr
     */
    DefaultDatabasePtr getDatabase(DatabaseID dbId);


    /**
     * Add a new and empty temporary set
     */
    bool addTempSet(std::string setName, SetID& setId, size_t pageSize = DEFAULT_PAGE_SIZE, size_t desiredSize = 1);


    /**
     * Remove an existing temporary set
     */
    bool removeTempSet(SetID setId);


    /**
     * Returns a temporary set specified
     */
    TempSetPtr getTempSet(SetID setId);


    /**
     * Returns a set specified
     */
    SetPtr getSet(DatabaseID dbId, UserTypeID typeId, SetID setId);


    /**
     * Start flushing main threads, which are also consumer threads,
     * to flush data in the flush buffer to disk files.
     */
    void startFlushConsumerThreads();


    /**
     * Stop flushing main threads, and close flushBuffer.
     */
    void stopFlushConsumerThreads();


    /**
     * returns a worker from thread pool
     */
    PDBWorkerPtr getWorker();

    /**
     * return whether the PangeaStorageServer instance is running standalone or in cluster mode.
     */
    bool isStandalone();


    // cleaner to be invoked in destruct

    void cleanup(bool flushOrNot = true);

    // export to a local file
    bool exportToFile(std::string dbName,
                      std::string setName,
                      std::string path,
                      std::string format,
                      std::string& errMsg);

    // export to a HDFS partition
    bool exportToHDFSFile(std::string dbName,
                          std::string setName,
                          std::string hdfsNameNodeIp,
                          int hdfsNameNodePort,
                          std::string path,
                          std::string format,
                          std::string& errMsg);


   // print all set information
   void printSets();

protected:
    /**
     * Encode database path
     */
    string encodeDBPath(string rootPath, DatabaseID dbId, string dbName);


    /**
     * Create temp directories
     */
    void createTempDirs();


    /**
     * Create root directories
     */
    void createRootDirs();


    /**
     * Initialize storage from existing root directories
     */
    bool initializeFromRootDirs(string metaRootPath, vector<string> dataRootPath);


    /**
     * Clear db data and disk files for removal
     */
    void clearDB(DatabaseID dbId, string dbName);


    /**
     * Add an existing database based on Sequence file (deprecated)
     */
    void addDatabaseBySequenceFiles(string dbName, DatabaseID dbId, boost::filesystem::path dbPath);


    /**
     * Add an existing database based on Partitioned file (by default)
     */
    void addDatabaseByPartitionedFiles(string dbName,
                                       DatabaseID dbId,
                                       boost::filesystem::path dbMetaPath);


private:
    // Mapping DatabaseID and SetID to a Set instance
    std::map<std::pair<DatabaseID, SetID>, SetPtr>* userSets;

    // Mapping Database name and Set name to DatabaseID and SetID
    std::map<std::pair<std::string, std::string>, std::pair<DatabaseID, SetID>>* names2ids;

    // Mapping a user type name to UserTypeID
    std::map<std::string, UserTypeID>* typename2id;

    // Mapping SetID to a TempSet instance
    std::map<SetID, TempSetPtr>* tempSets;

    // Mapping TempSet name to ID
    std::map<std::string, SetID>* name2tempSetId;

    // Log instance
    PDBLoggerPtr logger;

    // Configuration object
    ConfigurationPtr conf;

    // the name of the server
    std::string serverName;

    // the id of the server
    NodeID nodeId;

    // the instance to shared memory manager
    SharedMemPtr shm;

    // the path for storing TempSet metadata
    std::string metaTempPath;

    // the path for storing TempSet data
    std::vector<std::string> dataTempPaths;

    // the path for storing UserSet metadata
    std::string metaRootPath;

    // the path for storing UserSet data
    std::vector<std::string> dataRootPaths;

    // Pointer to page cache
    PageCachePtr cache;

    // Path to backend server
    string pathToBackEndServer;

    // mutex for managing database
    pthread_mutex_t databaseLock;

    // mutex for managing type
    pthread_mutex_t typeLock;

    // mutex for managing set
    pthread_mutex_t usersetLock;

    // mutex for managing tempset
    pthread_mutex_t tempsetLock;

    // SequenceID for adding temp set
    SequenceID tempsetSeqId;

    // SequenceID for adding database
    SequenceID databaseSeqId;

    // SequenceID for adding user set
    std::map<std::string, SequenceID*>* usersetSeqIds;

    // Thread Pool for starting flushing threads
    PDBWorkerQueuePtr workers;

    // the flush buffer connecting producers and consumers
    PageCircularBufferPtr flushBuffer;

    // The vector of flush threads
    std::vector<PDBWorkPtr> flushers;

    /****** for distribution *******************************/

private:
    bool standalone = true;


    /******* to comply with Chris' interfaces ***************/

private:
    // this stores the set of all records that we are buffering
    std::map<pair<std::string, std::string>, std::vector<Record<Vector<Handle<Object>>>*>>
        allRecords;

    // this stores the total sizes of all lists of records that we are buffering
    std::map<pair<std::string, std::string>, size_t> sizes;

    /******* backward compliance ***************************/

private:
    // this stores the set of databases
    std::map<DatabaseID, DefaultDatabasePtr>* dbs;

    // this stores the mapping from Database name to DatabaseID
    std::map<std::string, DatabaseID>* name2id;

    long totalObjects;

    pthread_mutex_t workingMutex;
    pthread_mutex_t counterMutex;
    int numWaitingBufferDataRequests;
    double inflationFactor = 40.0;
};
}

#endif
