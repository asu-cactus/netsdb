#ifndef OBJECTQUERYMODEL_BROADCASTSERVER_CC
#define OBJECTQUERYMODEL_BROADCASTSERVER_CC

#include "BroadcastServer.h"
namespace pdb {

BroadcastServer::BroadcastServer(PDBLoggerPtr logger, ConfigurationPtr conf) {
    this->conf = conf;
    pthread_mutex_init(&connection_mutex, nullptr);
}


BroadcastServer::BroadcastServer(PDBLoggerPtr logger) {
    this->logger = logger;
    this->conf = nullptr;
    pthread_mutex_init(&connection_mutex, nullptr);
}

BroadcastServer::~BroadcastServer() {
    pthread_mutex_destroy(&connection_mutex);
}

void BroadcastServer::registerHandlers(PDBServer& forMe) {
    // no-op
}
}

#endif
