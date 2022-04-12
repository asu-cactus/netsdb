
#ifndef PDBCLIENT_CC
#define PDBCLIENT_CC

#include "PDBClient.h"

namespace pdb {

PDBClient::PDBClient() {}

PDBClient::PDBClient(int portIn, std::string addressIn, PDBLoggerPtr myLoggerIn,
                     bool usePangaeaIn, bool useQuerySchedulerIn, std::string clientName)
    : port(portIn), address(addressIn), logger(myLoggerIn),
      usePangea(usePangaeaIn), useQueryScheduler(useQuerySchedulerIn), 
      clientName(clientName) {

  catalogClient = pdb::CatalogClient(
      portIn, addressIn, make_shared<pdb::PDBLogger>("catalogClientLog"));

  dispatcherClient = DispatcherClient(
      portIn, addressIn, make_shared<pdb::PDBLogger>("dispatcherClientLog"));

  distributedStorageClient = DistributedStorageManagerClient(
      portIn, addressIn,
      make_shared<pdb::PDBLogger>("distributedStorageClientLog"));

  queryClient = pdb::QueryClient(portIn, addressIn,
                                 make_shared<pdb::PDBLogger>("queryClientLog"),
                                 useQuerySchedulerIn, clientName);
}

PDBClient::~PDBClient() {}

pdb::CatalogClient PDBClient::getCatalogClient() { return catalogClient; }

pdb::DispatcherClient PDBClient::getDispatcherClient() {
  return dispatcherClient;
}

pdb::DistributedStorageManagerClient PDBClient::getDistributedStorageClient() {
  return distributedStorageClient;
}

pdb::QueryClient PDBClient::getQueryClient() { return queryClient; }

void PDBClient::registerHandlers(PDBServer &forMe) {}

/****
 * Methods for invoking DistributedStorageManager-related operations
 */
bool PDBClient::createDatabase(const std::string &databaseName,
                               std::string &errMsg) {

  return distributedStorageClient.createDatabase(databaseName, errMsg);
}


bool PDBClient::createSet(const std::string &databaseName,
                          const std::string &setName,
                          const std::string &typeName, std::string &errMsg,
                          size_t pageSize, const std::string &createdJobId, Handle<Computation> dispatchComputation, Handle<LambdaIdentifier> lambda, bool isSharedTensorBlockSet) {

  return distributedStorageClient.createSet(databaseName, setName, typeName,
                                            errMsg, pageSize, createdJobId, dispatchComputation, lambda, 0, false, isSharedTensorBlockSet);
}


bool PDBClient::createSet(const std::string &databaseName,
                          const std::string &setName,
                          const std::string &typeName, std::string &errMsg,
                          size_t pageSize, const std::string &createdJobId, Handle<Vector<Handle<Computation>>> dispatchComputations, std::string jobName, std::string jobName1, std::string jobName2, std::string computationName1, std::string computationName2, std::string lambdaName1, std::string lambdaName2, bool isSharedTensorBlockSet) {

  return distributedStorageClient.createSet(databaseName, setName, typeName,
                                            errMsg, pageSize, createdJobId, dispatchComputations, jobName, jobName1, jobName2, computationName1, computationName2, lambdaName1, lambdaName2, 0, false, isSharedTensorBlockSet);
}


bool PDBClient::createTempSet(const std::string &databaseName,
                              const std::string &setName,
                              const std::string &typeName, std::string &errMsg,
                              size_t pageSize, const std::string &createdJobId, Handle<Computation> dispatchComputation, Handle<LambdaIdentifier> lambda) {

  return distributedStorageClient.createTempSet(databaseName, setName, typeName,
                                                errMsg, pageSize, createdJobId, dispatchComputation, lambda);
}

bool PDBClient::removeDatabase(const std::string &databaseName,
                               std::string &errMsg) {

  return distributedStorageClient.removeDatabase(databaseName, errMsg);
}

bool PDBClient::removeSet(const std::string &databaseName,
                          const std::string &setName, std::string &errMsg) {

  return distributedStorageClient.removeSet(databaseName, setName, errMsg);
}

bool PDBClient::removeTempSet(const std::string &databaseName,
                              const std::string &setName,
                              const std::string &typeName,
                              std::string &errMsg) {

  return distributedStorageClient.removeTempSet(databaseName, setName, typeName,
                                                errMsg);
}

bool PDBClient::removeHashSet(const std::string &hashSetName,
                              std::string &errMsg) {

  return distributedStorageClient.removeHashSet(hashSetName,
                                                errMsg);
}

bool PDBClient::exportSet(const std::string &databaseName,
                          const std::string &setName,
                          const std::string &outputFilePath,
                          const std::string &format, std::string &errMsg) {

  return distributedStorageClient.exportSet(databaseName, setName,
                                            outputFilePath, format, errMsg);
}

bool PDBClient::clearSet(const std::string &databaseName,
                         const std::string &setName,
                         const std::string &typeName, std::string &errMsg) {

  return distributedStorageClient.clearSet(databaseName, setName, typeName,
                                           errMsg);
}


bool PDBClient::addSharedPage(std::string sharingDatabase,
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
                   std::string& errMsg) {
   
  return distributedStorageClient.addSharedPage(sharingDatabase, sharingSetName, sharingTypeName,
		  sharedDatabase, sharedSetName, sharedTypeName, pageId, filePartitionId, pageSeqId,
		  whetherToAddSharedSet, nodeId, errMsg);

}


bool PDBClient::addSharedMapping(std::string sharingDatabase,
                  std::string sharingSetName,
                  std::string sharingTypeName,
                  std::string sharedDatabase,
                  std::string sharedSetName,
                  std::string sharedTypeName,
                  std::string fileName,
                  size_t totalRows,
                  size_t totalCols,
		  bool transpose,
		  std::string errMsg) {
  return distributedStorageClient.addSharedMapping(sharingDatabase, sharingSetName, sharingTypeName,
                  sharedDatabase, sharedSetName, sharedTypeName, fileName, totalRows, totalCols, transpose, errMsg);

}

bool PDBClient::flushData(std::string &errMsg) {

  return distributedStorageClient.flushData(errMsg);
}

std::function<bool(Handle<SimpleRequestResult>)>
PDBClient::generateResponseHandler(std::string description,
                                   std::string &errMsg) {

  return [&](Handle<SimpleRequestResult> result) {
    if (result != nullptr) {
      if (!result->getRes().first) {
        errMsg = description + result->getRes().second;
        logger->error(description + ": " + result->getRes().second);
        return false;
      }
      return true;
    }
    errMsg = "Received nullptr as response";
    logger->error(description + ": received nullptr as response");
    return false;
  };
}

/****
 * Methods for invoking Catalog-related operations
 */

bool PDBClient::registerNode(string &localIP, int localPort, string &nodeName,
                             string &nodeType, int nodeStatus,
                             std::string &errMsg) {
      makeObjectAllocatorBlock(1024 * 1024 * 1, true);
      pdb::Handle<pdb::CatSyncRequest> nodeInfo = pdb::makeObject<pdb::CatSyncRequest>(localIP, localPort, nodeType);
      std::string returnedMsg="";
      bool result = catalogClient.registerNodeMetadata(nodeInfo, returnedMsg);
      if (!result) {
          errMsg = "Not able to register node: " + returnedMsg;
          exit(-1);
      } else {
          cout << "Node has been registered.\n";
      }
      return result;
}

bool PDBClient::registerType(std::string fileContainingSharedLib,
                             std::string &errMsg) {

  return catalogClient.registerType(fileContainingSharedLib, errMsg);
}

string PDBClient::printCatalogMetadata(
    pdb::Handle<pdb::CatPrintCatalogRequest> itemToSearch, std::string &errMsg) {

   return catalogClient.printCatalogMetadata (
      itemToSearch,
      errMsg) ;
}

string PDBClient::listAllRegisteredMetadata(std::string &errMsg) {
   return catalogClient.listAllRegisteredMetadata(errMsg);
}
 
string PDBClient::listRegisteredDatabases(std::string &errMsg) {
  return catalogClient.listRegisteredDatabases(errMsg);
}

string PDBClient::listRegisteredSetsForADatabase(std::string databaseName,
                                                 std::string &errMsg) {

  return catalogClient.listRegisteredSetsForADatabase(databaseName, errMsg);
}

string PDBClient::listNodesInCluster(std::string &errMsg) {
  return catalogClient.listNodesInCluster(errMsg);
}

string PDBClient::listUserDefinedTypes(std::string &errMsg) {
  return catalogClient.listUserDefinedTypes(errMsg);
}

/****
 * Methods for invoking Dispatcher-related operations
 */

bool PDBClient::registerSet(std::pair<std::string, std::string> setAndDatabase,
                            PartitionPolicy::Policy policy,
                            std::string &errMsg) {

  return dispatcherClient.registerSet(setAndDatabase, policy, errMsg);
}

/****
 * Methods for invoking Query-related operations
 */
bool PDBClient::deleteSet(std::string databaseName, std::string setName) {

  return queryClient.deleteSet(databaseName, setName);
}
}

#endif
