
#ifndef CAT_CLIENT_TEMPL_CC
#define CAT_CLIENT_TEMPL_CC

#include "CatalogClient.h"
#include "CatCreateSetRequest.h"
#include "SimpleRequest.h"
#include "SimpleRequestResult.h"

namespace pdb {

template <class DataType>
bool CatalogClient::createSet(std::string databaseName, std::string setName,
                              std::string &errMsg) {

  int16_t typeID = VTableMap::getIDByName(getTypeName<DataType>(), false);
  if (typeID == -1) {
    errMsg = "Could not find type " + getTypeName<DataType>();
    return -1;
  }

  return simpleRequest<CatCreateSetRequest, SimpleRequestResult, bool>(
      myLogger, port, address, false, 1024,
      [&](Handle<SimpleRequestResult> result) {
        if (result != nullptr) {
          if (!result->getRes().first) {
            errMsg = "Error creating set: " + result->getRes().second;
            myLogger->error("Error creating set: " + result->getRes().second);
            return false;
          }
          return true;
        }
        errMsg = "Error getting type name: got nothing back from catalog";
        return false;
      },
      databaseName, setName, typeID);
}
}

#endif
