
#define NUM_OBJECTS 12000

#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctime>
#include <string>

#include "Handle.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "Employee.h"
#include "Supervisor.h"

using namespace pdb;

void myFunc(String myIp) {
    String ip = myIp;
    String nodeAddress = std::string(ip) + ":8108";
    std::cout << "ip in myFunc is " << ip << std::endl;
    std::cout << "nodeAddress in myFunc is " << nodeAddress << std::endl;
}

int main() {
    std::string input = "localhost";
    String ip = String(input);
    myFunc(ip);
}
