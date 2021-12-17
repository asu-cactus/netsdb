#ifndef PDBCLIENT_H
#define PDBCLIENT_H

#include "CatalogClient.h"
#include "DispatcherClient.h"
#include "DistributedStorageManagerClient.h"
#include "QueryClient.h"
#include "ServerFunctionality.h"

#include "Handle.h"
#include "PDBObject.h"
#include "PDBVector.h"
#include "PartitionPolicy.h"

#include "SimpleRequest.h"

/**
 * This class provides functionality so users can connect and access
 * different Server functionalities of PDB, such as:
 *
 *  Catalog services
 *  Dispatcher services
 *  Distributed Storage services
 *  Query services
 *
 */

namespace pdb {

class PDBClient : public ServerFunctionality {

public:
  /* Creates PDBClient
   *
   *            portIn: the port number of the PDB master server
   *         addressIn: the IP address of the PDB master server
   *        myLoggerIn: the logger
   *         usePangea: true if Pangea is used
   * useQueryScheduler: true if Query Scheduler is used
   *        clientName: name of query client
   */
  PDBClient(int portIn, std::string addressIn, PDBLoggerPtr myLoggerIn,
            bool usePangeaIn, bool useQuerySchedulerIn, std::string clientName="");

  PDBClient();

  ~PDBClient();

  void registerHandlers(PDBServer &forMe); // no - op

  /****
   * Gets a reference to the Catalog Client
   */
  pdb::CatalogClient getCatalogClient();

  /****
   * Gets a reference to the Dispatcher Client
   */
  pdb::DispatcherClient getDispatcherClient();

  /****
   * Gets a reference to the Distributed Storage Manager Client
   */
  pdb::DistributedStorageManagerClient getDistributedStorageClient();

  /****
   * Gets a reference to the Query Client
   */
  pdb::QueryClient getQueryClient();

  /****
   * Methods for invoking DistributedStorageManager-related operations
   */

  /* Creates a database */
  bool createDatabase(const std::string &databaseName, std::string &errMsg);



  /* Creates a set with a given type for an existing database, which can be applied with lambda-based dispatching policy */
  bool createSet(const std::string &databaseName, const std::string &setName,
                 const std::string &typeName, std::string &errMsg,
                 size_t pageSize = DEFAULT_PAGE_SIZE, const std::string &createdJobId = "", Handle<Computation> dispatchComputation = nullptr, Handle<LambdaIdentifier> lambda = nullptr, bool isSharedTensorBlockSet = false);

  /* Creates a set with a given type for an existing database, which can be applied with IR-based dispatching policy*/
  bool createSet(const std::string &databaseName, const std::string &setName,
                 const std::string &typeName, std::string &errMsg,
                 size_t pageSize, const std::string &createdJobId, Handle<Vector<Handle<Computation>>> dispatchComputations, const std::string jobName, std::string jobName1, std::string jobName2, std::string computationName1, std::string computationName2, std::string lambdaName1, std::string lambdaName2, bool isSharedTensorBlockSet = false);


  /* Creates a set with a given type (using a template) for an existing
   * database with page_size value. */
  template <class DataType>  
  bool createSet(const std::string &databaseName,
                            const std::string &setName, std::string &errMsg, 
                            size_t pageSize = DEFAULT_PAGE_SIZE,
                            const std::string &createdJobId = "", Handle<Computation> dispatchComputation = nullptr, Handle<LambdaIdentifier> lambda = nullptr, bool isSharedTensorBlockSet = false);

 /* Creates a set with a given type (using a template) for an existing
   * database with page_size value, which can be applied with IR-based dispatching policy */
  template <class DataType>
  bool createSet(const std::string &databaseName,
                            const std::string &setName, std::string &errMsg,
                            size_t pageSize,
                            const std::string &createdJobId, Handle<Vector<Handle<Computation>>> dispatchComputations, const std::string jobName, std::string jobName1, std::string jobName2, std::string computationName1, std::string computationName2, std::string lambdaName1, std::string lambdaName2, bool isSharedTensorBlockSet = false);

  /* Creates a temporary set with a given type for an existing database (only
   * goes through storage) */
  bool createTempSet(const std::string &databaseName,
                     const std::string &setName, const std::string &typeName,
                     std::string &errMsg, size_t pageSize = DEFAULT_PAGE_SIZE, 
                     const std::string &createdJobId = "", Handle<Computation> dispatchComputation = nullptr, Handle<LambdaIdentifier> lambda = nullptr);

  /* Link shared page to a private set at a specific node*/
  bool addSharedPage(std::string sharingDatabase,
                   std::string sharingSetName,
                   std::string sharingTypeName,
                   std::string sharedDatabase,
                   std::string sharedSetName,
                   std::string sharedTypeName,
                   PageID pageId,
                   FilePartitionID filePartitionId,
                   unsigned int pageSeqId,
                   bool whetherToAddSharedSet,
                   NodeID nodeId,
                   std::string& errMsg); 

  /* Flushes data currently in memory into disk. */
  bool flushData(std::string &errMsg);

  /* Removes a database from storage. */
  bool removeDatabase(const std::string &databaseName, std::string &errMsg);

  /* Removes a set for an existing database from storage. */
  bool removeSet(const std::string &databaseName, const std::string &setName,
                 std::string &errMsg);


  /* Removes a hash set from HermesExecutionServer. */
  bool removeHashSet(const std::string &hashSetName, std::string &errMsg);


  /* Removes a set given a type from an existing database. */
  bool clearSet(const std::string &databaseName, const std::string &setName,
                const std::string &typeName, std::string &errMsg);

  /* Removes a temporary set given a type from an existing database (only goes
   * through storage). */
  bool removeTempSet(const std::string &databaseName,
                     const std::string &setName, const std::string &typeName,
                     std::string &errMsg);

  /* Exports the content of a set from a database into a file. Note that the
   * objects in
   * set must be instances of ExportableObject
   */
  bool exportSet(const std::string &databaseName, const std::string &setName,
                 const std::string &outputFilePath, const std::string &format,
                 std::string &errMsg);

  /****
   * Methods for invoking Catalog-related operations
   */

  /* Sends a request to the Catalog Server to register a user-defined type
   * defined
   * in a shared library. */
  bool registerType(std::string fileContainingSharedLib, std::string &errMsg);

  /* Sends a request to the Catalog Server to register metadata about a node in
   * the cluster */
  bool registerNode(string &localIP, int localPort, string &nodeName,
                    string &nodeType, int nodeStatus, std::string &errMsg);

  /* Prints the content of the catalog. */
  string
  printCatalogMetadata(pdb::Handle<pdb::CatPrintCatalogRequest> itemToSearch,
                       std::string &errMsg);

  /* Lists all metadata registered in the catalog. */
  string listAllRegisteredMetadata(std::string &errMsg);

  /* Lists the Databases registered in the catalog. */
  string listRegisteredDatabases(std::string &errMsg);

  /* Lists the Sets for a given database registered in the catalog. */
  string listRegisteredSetsForADatabase(std::string databaseName,
                                        std::string &errMsg);

  /* Lists the Nodes registered in the catalog. */
  string listNodesInCluster(std::string &errMsg);

  /* Lists user-defined types registered in the catalog. */
  string listUserDefinedTypes(std::string &errMsg);

  /****
   * Methods for invoking Dispatcher-related operations
   *
   * @param setAndDatabase
   * @return
   */
  bool registerSet(std::pair<std::string, std::string> setAndDatabase,
                   PartitionPolicy::Policy policy, std::string &errMsg);

  /**
   *
   * @param setAndDatabase
   * @return
   */
  template <class DataType>
  bool sendData(std::pair<std::string, std::string> setAndDatabase,
                Handle<Vector<Handle<DataType>>> dataToSend,
                std::string &errMsg);

  template <class DataType>
  bool sendBytes(std::pair<std::string, std::string> setAndDatabase,
                 char *bytes, size_t numBytes, std::string &errMsg);

  /****
   * Methods for invoking Query-related operations
   */

  /* Executes some computations */
  template <class... Types>
  bool executeComputations(std::string &errMsg, Handle<Computation> firstParam,
                           Handle<Types>... args);


  template <class... Types>
  bool executeComputations(std::string &errMsg, std::string jobName, Handle<Computation> firstParam,
                           Handle<Types>... args);


  template <class... Types>
  bool executeComputations(std::string &errMsg, std::string jobName, bool preCompile, Handle<Computation> firstParam,
                           Handle<Types>... args);



  /* Deletes a set. */
  bool deleteSet(std::string databaseName, std::string setName);

  /* Gets a set iterator. */
  template <class Type>
  SetIterator<Type> getSetIterator(std::string databaseName,
                                   std::string setName, 
				   bool isShared = false);


  static std::string getNextClientId() {
        time_t currentTime = time(NULL);
        struct tm* local = localtime(&currentTime);
        std::string clientId = "Client-" + std::to_string(local->tm_year + 1900) + "_" +
            std::to_string(local->tm_mon + 1) + "_" + std::to_string(local->tm_mday) + "_" +
            std::to_string(local->tm_hour) + "_" + std::to_string(local->tm_min) + "_" +
            std::to_string(local->tm_sec);
        return clientId;
  }


public:
  pdb::CatalogClient catalogClient;
  pdb::DispatcherClient dispatcherClient;
  pdb::DistributedStorageManagerClient distributedStorageClient;
  pdb::QueryClient queryClient;

  std::function<bool(Handle<SimpleRequestResult>)>
  generateResponseHandler(std::string description, std::string &errMsg);

  int port;
  bool usePangea;
  bool useQueryScheduler;
  std::string clientName;
  std::string address;
  PDBLoggerPtr logger;
};
}

#include "DispatcherClientTemplate.cc"
#include "PDBClientTemplate.cc"
#include "StorageClientTemplate.cc"

#endif
