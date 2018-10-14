#include <iostream>
#include "PDBServer.h"

using namespace pdb;

void logToFile(const pdb::PDBLoggerPtr &logger) {
    logger->fatal("This is a FATAL");
    logger->error("This is a ERROR");
    logger->info("This is a INFO");
    logger->debug("This is a DEBUG");
    logger->trace("This is a TRACE");
}


int main() {


    std::string backendLoggerFile = "myLogFile.log";
    pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>(backendLoggerFile);

    logger->fatal("This is a SPARATOR ======   Log Level FATAL ===================  ");
    logger->setLoglevel(FATAL);
    logToFile(logger);


    logger->fatal("This is a SPARATOR ======   Log Level ERROR ===================  ");
    logger->setLoglevel(ERROR);
    logToFile(logger);


    logger->fatal("This is a SPARATOR ======   Log Level INFO ===================  ");
    logger->setLoglevel(INFO);
    logToFile(logger);


    logger->fatal("This is a SPARATOR ======   Log Level DEBUG ===================  ");
    logger->setLoglevel(DEBUG);
    logToFile(logger);


    logger->fatal("This is a SPARATOR ======   Log Level TRACE ===================  ");
    logger->setLoglevel(TRACE);
    logToFile(logger);

    return 0;
}
