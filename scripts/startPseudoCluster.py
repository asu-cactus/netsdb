#!/usr/bin/python3
#to run pseudo cluster on single node

import subprocess
import time
import sys

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

threadNum = "1"
sharedMemorySize = "4096"
batchSize = 1

if(len(sys.argv)==3) :
    threadNum = sys.argv[1]
    sharedMemorySize = sys.argv[2]
elif(len(sys.argv)==4) :
    threadNum = sys.argv[1]
    sharedMemorySize = sys.argv[2]
    batchSize = sys.argv[3]
else:
    print (bcolors.OKBLUE+"Usage: python scripts/startPseduoCluster.py numThreads (default: 1) sizeOfSharedMemoryPool (default: 2048 MB) batchSize (optional default: 1)")


print("#################################")
print("RUN A PSEDO CLUSTER")
print("#################################")

try:
    #run bin/pdb-cluster
    print (bcolors.OKBLUE + "start a pdbServer as the coordinator" + bcolors.ENDC)
    serverProcess = subprocess.Popen(['bin/pdb-cluster', 'localhost', '8108', 'Y'])
    print (bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC)
    time.sleep(9)
    #strip shared libraries
    print (bcolors.OKBLUE + "strip shared libraries" + bcolors.ENDC)
    subprocess.call(['bash', 'strip libraries/*.so'])

    #run bin/pdb-server for worker
    num = 0;
    with open('conf/serverlist.test') as f:
        for each_line in f:
            print (bcolors.OKBLUE + "start a pdbServer at " + each_line + "as " + str(num) + "-th worker" + bcolors.ENDC)
            num = num + 1
            serverProcess = subprocess.Popen(['bin/pdb-server', str(num-1), threadNum, sharedMemorySize, 'localhost:8108', each_line, batchSize])
            print (bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC)
            time.sleep(9)


except subprocess.CalledProcessError as e:
    print (bcolors.FAIL + "[ERROR] in starting peudo cluster" + bcolors.ENDC)
    print (e.returncode)