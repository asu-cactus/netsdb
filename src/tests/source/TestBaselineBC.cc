#ifndef TEST_BASELINE_BC_CC
#define TEST_BASELINE_BC_CC

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "PDBClient.h"
#include "StorageClient.h"
#include "PDBClient.h"
#include "BaselineNode.h"
#include "MyPrediction.h"

using namespace pdb;

int main(int argc, char *argv[]) {

	string errMsg;

	makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

	string masterIp = "localhost";
	pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
	pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
	pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);
	
	return 0;
}

#endif