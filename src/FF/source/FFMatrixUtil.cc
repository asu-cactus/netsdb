#include "FFMatrixUtil.h"
#include "FFMatrixBlock.h"
#include "PDBClient.h"
#include "TensorBlock2D.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <random>
using namespace std;

namespace ff {
void load_matrix_data(pdb::PDBClient &pdbClient, string path,
                      pdb::String dbName, pdb::String setName, int blockX,
                      int blockY, bool dont_pad_x, bool dont_pad_y,
                      string &errMsg, int size, bool partitionByCol) {
    if (path.size() == 0) {
        throw runtime_error("Invalid filepath: " + path);
    }

    int totalX, totalY;

    /// 1. Load the data from the file

    // open the input file
    ifstream is(path);
    while (is.peek() == '#' || is.peek() == ' ')
        is.ignore();

    // load the data stats
    is >> totalX;

    while (is.peek() == ',' || is.peek() == ' ')
        is.ignore();

    is >> totalY;

    cout << totalX << ", " << totalY << endl;

    vector<vector<double>> matrix;

    double val;
    int total = 0;
    pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

    pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
        pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

    int numXBlocks = ceil(totalX / (double)blockX);
    int numYBlocks = ceil(totalY / (double)blockY);

    int i = 0;
    int j = 0;
    int ii = 0;
    int jj = 0;

    while (i < numXBlocks) {

        try {
            while (i < numXBlocks) {
                int actual_blockX =
                    dont_pad_x ? min(blockX, totalX - i * blockX) : blockX;

                while (j < numYBlocks) {
                    int actual_blockY =
                        dont_pad_y ? min(blockY, totalY - j * blockY) : blockY;

                    pdb::Handle<FFMatrixBlock> myData =
                        pdb::makeObject<FFMatrixBlock>(i, j, actual_blockX, actual_blockY,
                                                       totalX, totalY, partitionByCol);

                    while (ii < actual_blockX) {
                        while (jj < actual_blockY) {
                            int curX = (i * actual_blockX + ii);
                            int curY = (j * actual_blockY + jj);

                            if ((dont_pad_x && curX >= totalX) ||
                                (dont_pad_y && curY >= totalY)) {
                                cout << "Shouldnt be here!" << endl;
                                exit(1);
                            }

                            is >> val;
                            while (is.peek() == ',' || is.peek() == ' ')
                                is.ignore();

                            // row = i * blockX + ii, col = j * blockY + jj
                            double data = curX >= totalX || curY >= totalY ? 0 : val;
                            (*(myData->getRawDataHandle()))[ii * actual_blockY + jj] = data;
                            jj++;
                        }
                        ii++;
                        jj = 0;
                    }

                    // cout << "New block: " << total << endl;
                    storeMatrix1->push_back(myData);
                    total++;
                    j++;
                    ii = 0;
                    jj = 0;
                }

                i++;
                j = 0;
                ii = 0;
                jj = 0;
            }
            if (!pdbClient.sendData<FFMatrixBlock>(
                    pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
                cout << "Failed to send data to dispatcher server" << endl;
                exit(1);
            }
        } catch (pdb::NotEnoughSpace &e) {
            if (!pdbClient.sendData<FFMatrixBlock>(
                    pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
                cout << "Failed to send data to dispatcher server" << endl;
                exit(1);
            }
            std::cout << "Dispatched " << storeMatrix1->size() << " blocks."
                      << std::endl;
            pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);
            storeMatrix1 = pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();
        }
    }

    cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
         << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
         << blockY << " each" << endl;

    // to write back all buffered records
    pdbClient.flushData(errMsg);
}

void load_matrix_data(pdb::PDBClient &pdbClient, string path,
                      pdb::String dbName, pdb::String setName, int pad_x,
                      int pad_y, string &errMsg) {
    if (path.size() == 0) {
        throw runtime_error("Invalid filepath: " + path);
    }

    int totalX, totalY;

    /// 1. Load the data from the file

    // open the input file
    ifstream is(path);
    while (is.peek() == '#' || is.peek() == ' ')
        is.ignore();

    // load the data stats
    is >> totalX;

    while (is.peek() == ',' || is.peek() == ' ')
        is.ignore();

    is >> totalY;

    cout << totalX << ", " << totalY << endl;

    vector<vector<double>> matrix;

    double val;
    for (int i = 0; i < totalX; i++) {
        vector<double> line;
        for (int j = 0; j < totalY; j++) {
            is >> val;
            line.push_back(val);
            while (is.peek() == ',' || is.peek() == ' ')
                is.ignore();
        }
        matrix.push_back(line);
    }

    if (matrix.size() == 0) {
        throw runtime_error("Invalid matrix file: " + path);
    }

    int total = 0;
    pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);

    pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
        pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

    try {
        pdb::Handle<FFMatrixBlock> myData = pdb::makeObject<FFMatrixBlock>(
            0, 0, totalX + pad_x, totalY + pad_y, totalX + pad_x, totalY + pad_y);

        for (int i = 0; i < totalX + pad_x; i++) {
            for (int j = 0; j < totalY + pad_y; j++) {
                double data = i >= totalX || j >= totalY
                                  ? 0 // padding to adjust to dimensions
                                  : matrix[i][j];
                (*(myData->getRawDataHandle()))[i * (totalY + pad_y) + j] = data;
            }
        }

        storeMatrix1->push_back(myData);

        if (!pdbClient.sendData<FFMatrixBlock>(
                pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
            cout << "Failed to send data to dispatcher server" << endl;
            exit(1);
        }
    } catch (pdb::NotEnoughSpace &e) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
    }

    // to write back all buffered records
    pdbClient.flushData(errMsg);
}

template <class T>
inline void writeMyData(const pdb::Handle<T> &myData, ofstream &outFile, int blockY) {

    const auto &dataHandle = *(myData->getRawDataHandle());
    for (std::size_t i{0}; i < dataHandle.size();) {
        for (std::size_t index{0}; index < blockY - 1; index++) {
            outFile << dataHandle[i++] << ',';
        }
        outFile << std::setprecision(0) << dataHandle[i++] << '\n';
    }
}

template <class T>
int allocateMyDataHelper(pdb::Handle<T> &myData, int xBlockCounter, int numXBlocks, int blockX, int blockY,
                         int curPartitionSize, int totalY, int partitionByCol) {
    // If this is the last XBlock, the size may not be equal to blockX,
    // thus compute the remainder of curPartitionSize over blockX.
    int remainder = curPartitionSize % blockX;
    int curBlockRows = (xBlockCounter == numXBlocks - 1 && remainder != 0)
                           ? remainder
                           : blockX;
    myData = pdb::makeObject<T>(xBlockCounter, 0, curBlockRows, blockY,
                                curPartitionSize, totalY, partitionByCol);
    myData->print();
    return curBlockRows;
}

template <class T>
int allocateMyData(pdb::PDBClient &pdbClient, pdb::Handle<pdb::Vector<pdb::Handle<T>>> &storeMatrix, pdb::Handle<T> &myData,
                   std::string curSetName, std::string dbName, std::string &errMsg,
                   int xBlockCounter, int numXBlocks, int blockX, int blockY, int curPartitionSize,
                   int totalY, int partitionByCol, int size) {
    int curBlockRows = blockX;
    try {
        curBlockRows = allocateMyDataHelper(myData, xBlockCounter, numXBlocks, blockX, blockY,
                                            curPartitionSize, totalY, partitionByCol);
    } catch (pdb::NotEnoughSpace &n) {
        if (!pdbClient.sendData<T>(
                pair<string, string>(curSetName, dbName), storeMatrix, errMsg)) {
            cout << "Failed to send data to dispatcher server" << endl;
            exit(1);
        }
        std::cout << "Dispatched " << storeMatrix->size() << " blocks to " << curSetName << '\n';
        storeMatrix.emptyOutContainingBlock();
        pdb::makeObjectAllocatorBlock((size_t)size * (size_t)1024 * (size_t)1024, true);
        storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
        curBlockRows = allocateMyDataHelper(myData, xBlockCounter, numXBlocks, blockX, blockY,
                                            curPartitionSize, totalY, partitionByCol);
    }
    return curBlockRows;
}

template <class T>
void allocateMyDataSparse(pdb::PDBClient &pdbClient, pdb::Handle<pdb::Vector<pdb::Handle<T>>> &storeMatrix, pdb::Handle<T> &myData,
                          std::string curSetName, std::string dbName, std::string &errMsg, int size) {
    try {
        myData = pdb::makeObject<pdb::Map<int, float>>(64);
    } catch (pdb::NotEnoughSpace &n) {
        if (!pdbClient.sendData<T>(
                pair<string, string>(curSetName, dbName), storeMatrix, errMsg)) {
            cout << "Failed to send data to dispatcher server" << endl;
            exit(1);
        }
        std::cout << "Dispatched " << storeMatrix->size() << " blocks to " << curSetName << '\n';
        storeMatrix.emptyOutContainingBlock();
        pdb::makeObjectAllocatorBlock((size_t)size * (size_t)1024 * (size_t)1024, true);
        storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
        myData = pdb::makeObject<pdb::Map<int, float>>(64);
    }
    return;
}

template <class T>
void pushBackMyDataToStoreMatrix(pdb::PDBClient &pdbClient, pdb::Handle<pdb::Vector<pdb::Handle<T>>> &storeMatrix, pdb::Handle<T> &myData,
                                 std::string curSetName, std::string dbName, std::string &errMsg, int size, int total) {
    try {
        storeMatrix->push_back(myData);
    } catch (pdb::NotEnoughSpace &n) {
        if (!pdbClient.sendData<T>(
                pair<string, string>(curSetName, dbName), storeMatrix, errMsg)) {
            std::cout << "Failed to send data to dispatcher server\n";
            exit(1);
        }
        std::cout << "Dispatched " << storeMatrix->size() << " objects to " << curSetName << '\n';
        storeMatrix.emptyOutContainingBlock();
        pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);
        storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
        storeMatrix->push_back(myData);
    }
    std::cout << "Stored " << total << " items in total.\n";
}

template <class T>
void processOneCsvLine(pdb::Handle<T> &myData, const std::string &line, int labelColIndex, int xBlockRowCounter, int blockY) {
    const int startOffset = xBlockRowCounter * blockY;
    int colIndex = 0;
    int myDataColIndex = 0;
    for (std::string::size_type pos = 0, newpos = 0; newpos != line.length() - 1;) {
        newpos = line.find(',', pos);
        if (newpos == std::string::npos) {
            // When this is evaluated true, the loop will break
            newpos = line.length() - 1;
        }

        if (colIndex != labelColIndex) {
            std::string token = line.substr(pos, newpos - pos);
            (*(myData->getRawDataHandle()))[startOffset + myDataColIndex] =
                token.empty() ? std::nan("") : std::stod(token.data());
            myDataColIndex++;
        }
        pos = newpos + 1;
        colIndex++;
    }
    assert(myDataColIndex == blockY);
}

template <class T>
void processOneSvmLine(pdb::Handle<T> &myData, const std::string &line, int labelColIndex, int xBlockRowCounter, int blockY, int curBlockRows) {
    // Set all value default to std::nan("")
    for (int i = 0; i < blockY * curBlockRows; i++) {
        (*(myData->getRawDataHandle()))[i] = std::nan("");
    }

    const int startOffset = xBlockRowCounter * blockY;
    std::string::size_type startIndex = line.find(' ') + 1;
    std::string::size_type midIndex = 0;
    std::string::size_type endIndex = 0;
    do {
        endIndex = line.find(' ', startIndex);
        const std::string feature = line.substr(startIndex, endIndex - startIndex);
        midIndex = feature.find(':');
        int featureIndex = std::stoi(feature.substr(0, midIndex));
        float featureValue = std::stof(feature.substr(midIndex + 1));
        (*(myData->getRawDataHandle()))[startOffset + featureIndex] = featureValue;
        startIndex = endIndex + 1;
    } while (endIndex != std::string::npos);
}

// TODO: Because adding a key-value pair to the Map may lead to doubling the pairarray size, we have to catch the NotEnoughSpaceException, just in case we do not know how many non-zero elements exist in a tuple.
void processOneSvmLineSparse(pdb::Map<int, float> &myData, const std::string &line) {

    std::string::size_type startIndex = line.find(' ') + 1;
    std::string::size_type midIndex = 0;
    std::string::size_type endIndex = 0;
    do {
        endIndex = line.find(' ', startIndex);
        const std::string feature = line.substr(startIndex, endIndex - startIndex);
        midIndex = feature.find(':');
        int featureIndex = std::stoi(feature.substr(0, midIndex));
        float featureValue = std::stof(feature.substr(midIndex + 1));
        myData[featureIndex] = featureValue;
        startIndex = endIndex + 1;
    } while (endIndex != std::string::npos);
}

// the number of columns in the input file should be equivalent to the blockY
void loadMapFromSVMFile(pdb::PDBClient &pdbClient, std::string path,
                        pdb::String dbName, pdb::String setName,
                        int totalX, int totalY,
                        std::string &errMsg, int size, int numPartitions, bool partitionByCol) {

    ifstream inFile(path);
    if (!inFile.is_open()) {
        std::cout << __FILE__ << ": Error: Input file [" << path << "] is not found!!!\n";
        exit(1);
    }

    pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

    int partitionSize = 0;
    int remainderPartitionSize = 0;
    if (numPartitions == 1) {
        partitionSize = totalX;
        remainderPartitionSize = totalX;
    } else {
        partitionSize = totalX / numPartitions;
        remainderPartitionSize = totalX - partitionSize * (numPartitions - 1);
    }
    std::cout << "partitionSize: " << partitionSize << "; remainderPartitionSize: " << remainderPartitionSize << std::endl;

    int total = 0;
    for (int curPartitionIndex = 0; curPartitionIndex < numPartitions; curPartitionIndex++) {
        int curPartitionSize = (curPartitionIndex == numPartitions - 1)
                                   ? remainderPartitionSize
                                   : partitionSize;

        std::cout << "current partition index: " << curPartitionIndex << '\n';

        std::string curSetName = std::string(setName);
        if (numPartitions > 1) {
            curSetName = curSetName + std::to_string(curPartitionIndex);
        }
        std::cout << "curSetName = " << curSetName << std::endl;

        pdb::Handle<pdb::Vector<pdb::Handle<pdb::Map<int, float>>>> storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<pdb::Map<int, float>>>>();
        std::string line;

        pdb::Handle<pdb::Map<int, float>> myData = nullptr;

        while (total < totalY) {
            // std::cout << "total = " << total << std::endl;
            // std::cout << "totalY = " << totalY << std::endl;
            if (!std::getline(inFile, line)) {
                std::cout << "We come to the end of the input file" << std::endl;
                if (myData != nullptr) {
                    pushBackMyDataToStoreMatrix(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, size, total);
                    myData = nullptr;
                }
                break;
            } else {
                // std::cout << line << std::endl;
                total++;
            }

            if (myData == nullptr) {
                allocateMyDataSparse(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, size);
            }

            processOneSvmLineSparse(*myData, line);

            pushBackMyDataToStoreMatrix(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, size, total);
            myData = nullptr;
        }

        if (storeMatrix->size() > 0) {
            if (!pdbClient.sendData<pdb::Map<int, float>>(
                    pair<string, string>(curSetName, dbName), storeMatrix, errMsg)) {
                std::cout << "Failed to send data to dispatcher server\n";
                exit(1);
            }
            std::cout << "Dispatched " << storeMatrix->size() << " maps to " << curSetName << '\n';
        }
    }

    // to write back all buffered records
    pdbClient.flushData(errMsg);
    inFile.close();
}

// the number of columns in the input file should be equivalent to the blockY
template <class T>
void loadMatrixGenericFromFile(pdb::PDBClient &pdbClient, std::string path,
                               pdb::String dbName, pdb::String setName,
                               int totalX, int totalY, int blockX, int blockY, int labelColIndex,
                               std::string &errMsg, int size, int numPartitions, bool partitionByCol) {

    std::cout << "blockX=" << blockX << std::endl;
    std::cout << "blockY=" << blockY << std::endl;
    std::cout << "totalX=" << totalX << std::endl;
    std::cout << "totalY=" << totalY << std::endl;
    std::cout << "size=" << size << std::endl;
    std::cout << "numPartitions=" << numPartitions << std::endl;

    // open the input file
    ifstream inFile(path);
    if (!inFile.is_open()) {
        std::cout << __FILE__ << ": Error: Input file [" << path << "] is not found!!!\n";
        exit(1);
    }
    const std::string fileSuffix = path.substr(path.rfind('.'));
    // ofstream outFile;
    // outFile.open("model-inference/decisionTree/experiments/dataset/HIGGS_out_test.csv");
    // if (!outFile.is_open()) {
    //     std::cout << __FILE__ << "Error: Output file is not found!!!\n";
    //     exit(1);
    // }

    pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

    int partitionSize = 0;
    int remainderPartitionSize = 0;
    if (numPartitions == 1) {
        partitionSize = totalX;
        remainderPartitionSize = totalX;
    } else {
        partitionSize = totalX / numPartitions;
        remainderPartitionSize = totalX - partitionSize * (numPartitions - 1);
    }
    std::cout << "partitionSize: " << partitionSize << "; remainderPartitionSize: " << remainderPartitionSize << std::endl;

    int total = 0;
    for (int curPartitionIndex = 0; curPartitionIndex < numPartitions; curPartitionIndex++) {
        int curPartitionSize = (curPartitionIndex == numPartitions - 1)
                                   ? remainderPartitionSize
                                   : partitionSize;

        int numXBlocks = ceil(curPartitionSize / (double)blockX);

        std::cout << "current partition index: " << curPartitionIndex
                  << "; number of blocks within the current partition: " << numXBlocks << '\n';

        std::string curSetName = std::string(setName);
        if (numPartitions > 1) {
            curSetName = curSetName + std::to_string(curPartitionIndex);
        }
        std::cout << "curSetName = " << curSetName << std::endl;

        pdb::Handle<pdb::Vector<pdb::Handle<T>>> storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
        pdb::Handle<T> myData = nullptr;
        std::string line;
        int xBlockCounter = 0;
        int xBlockRowCounter = 0;

        int curBlockRows = blockX;

        while (true) {
            if (xBlockCounter >= numXBlocks)
                break;
            // Each iteration of the while loop process one line of the input file
            if (!std::getline(inFile, line)) {
                std::cout << "We come to the end of the input file" << std::endl;
                if (myData != nullptr) {
                    pushBackMyDataToStoreMatrix(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, size, total);
                    xBlockCounter++;
                    // writeMyData(myData, outFile, blockY);
                    myData = nullptr;
                }
                break;
            } else {
                total++;
            }

            if (myData == nullptr) {
                curBlockRows = allocateMyData(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, xBlockCounter, numXBlocks,
                                              blockX, blockY, curPartitionSize, totalY, partitionByCol, size);
            }

            // Process one line of the input file.
            if (fileSuffix == ".csv") {
                processOneCsvLine(myData, line, labelColIndex, xBlockRowCounter, blockY);
            } else if (fileSuffix == ".svm") {
                processOneSvmLine(myData, line, labelColIndex, xBlockRowCounter, blockY, curBlockRows);
            } else {
                std::cout << "Cannot process file with suffix: " << fileSuffix << std::endl;
                exit(1);
            }

            xBlockRowCounter++;
            if (xBlockRowCounter == curBlockRows) {
                xBlockRowCounter = 0;
                pushBackMyDataToStoreMatrix(pdbClient, storeMatrix, myData, curSetName, dbName, errMsg, size, total);
                xBlockCounter++;
                // writeMyData(myData, outFile, blockY);
                myData = nullptr;
            }
        }

        if (storeMatrix->size() > 0) {
            if (!pdbClient.sendData<T>(
                    pair<string, string>(curSetName, dbName), storeMatrix, errMsg)) {
                std::cout << "Failed to send data to dispatcher server\n";
                exit(1);
            }
            std::cout << "Dispatched " << storeMatrix->size() << " blocks to " << curSetName << '\n';
        }
    }

    // to write back all buffered records
    pdbClient.flushData(errMsg);
    inFile.close();
    // outFile.close();
}

template <class T>
void loadMatrixGeneric(pdb::PDBClient &pdbClient, pdb::String dbName,
                       pdb::String setName, int totalX, int totalY, int blockX,
                       int blockY, bool dont_pad_x, bool dont_pad_y, string &errMsg,
                       int size, bool partitionByCol) {

    if ((totalX == 0) || (totalY == 0) || (blockX == 0) || (blockY == 0)) {

        return;
    }

    std::cout << "totalX=" << totalX << ", totalY=" << totalY
              << ",blockX=" << blockX << ", blockY=" << blockY << std::endl;
    std::random_device rd;

    std::mt19937 e2(rd());

    std::uniform_real_distribution<> distp(0.0001, 0.5);
    std::uniform_real_distribution<> distn(-0.5, -0.0001);

    auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                         std::default_random_engine());

    int total = 0;
    pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

    pdb::Handle<pdb::Vector<pdb::Handle<T>>> storeMatrix1 =
        pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();

    int numXBlocks = ceil(totalX / (double)blockX);
    int numYBlocks = ceil(totalY / (double)blockY);

    int i = 0;
    int j = 0;
    int ii = 0;
    int jj = 0;

    while (i < numXBlocks) {

        try {
            while (i < numXBlocks) {
                int actual_blockX =
                    dont_pad_x ? min(blockX, totalX - i * blockX) : blockX;

                while (j < numYBlocks) {
                    int actual_blockY =
                        dont_pad_y ? min(blockY, totalY - j * blockY) : blockY;

                    pdb::Handle<T> myData =
                        pdb::makeObject<T>(i, j, actual_blockX, actual_blockY,
                                           totalX, totalY, partitionByCol);

                    myData->print();

                    while (ii < actual_blockX) {
                        while (jj < actual_blockY) {
                            int curX = (i * actual_blockX + ii);
                            int curY = (j * actual_blockY + jj);

                            if ((dont_pad_x && curX >= totalX) ||
                                (dont_pad_y && curY >= totalY)) {
                                cout << "Shouldnt be here!" << endl;
                                exit(1);
                            }

                            // row = i * blockX + ii, col = j * blockY + jj
                            float data = curX >= totalX || curY >= totalY ? 0
                                         : (bool)gen()                    ? distn(e2)
                                                                          : distp(e2);
                            (*(myData->getRawDataHandle()))[ii * actual_blockY + jj] = data;
                            jj++;
                        }
                        ii++;
                        jj = 0;
                    }

                    std::cout << "New block: " << total << endl;
                    storeMatrix1->push_back(myData);
                    total++;
                    j++;
                    ii = 0;
                    jj = 0;
                }

                i++;
                j = 0;
                ii = 0;
                jj = 0;
            }
            if (!pdbClient.sendData<T>(
                    pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
                cout << "Failed to send data to dispatcher server" << endl;
                exit(1);
            }
        } catch (pdb::NotEnoughSpace &e) {
            if (!pdbClient.sendData<T>(
                    pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
                cout << "Failed to send data to dispatcher server" << endl;
                exit(1);
            }
            std::cout << "Dispatched " << storeMatrix1->size() << " blocks."
                      << std::endl;
            pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);
            storeMatrix1 = pdb::makeObject<pdb::Vector<pdb::Handle<T>>>();
        }
    }

    cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
         << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
         << blockY << " each" << endl;

    // to write back all buffered records
    pdbClient.flushData(errMsg);
}

void loadMatrix(pdb::PDBClient &pdbClient, pdb::String dbName,
                pdb::String setName, int totalX, int totalY, int blockX,
                int blockY, bool dont_pad_x, bool dont_pad_y, string &errMsg,
                int size, bool partitionByCol) {

    return loadMatrixGeneric<FFMatrixBlock>(pdbClient, dbName, setName, totalX, totalY, blockX,
                                            blockY, dont_pad_x, dont_pad_y, errMsg, size, partitionByCol);
}

template void loadMatrixGeneric<pdb::TensorBlock2D<float>>(pdb::PDBClient &pdbClient, pdb::String dbName,
                                                           pdb::String setName, int totalX, int totalY, int blockX,
                                                           int blockY, bool dont_pad_x, bool dont_pad_y, string &errMsg,
                                                           int size = 128, bool partitionByCol = true);

template void loadMatrixGeneric<pdb::TensorBlock2D<double>>(pdb::PDBClient &pdbClient, pdb::String dbName,
                                                            pdb::String setName, int totalX, int totalY, int blockX,
                                                            int blockY, bool dont_pad_x, bool dont_pad_y, string &errMsg,
                                                            int size = 128, bool partitionByCol = true);

template void loadMatrixGenericFromFile<pdb::TensorBlock2D<float>>(pdb::PDBClient &pdbClient, std::string filePath, pdb::String dbName,
                                                                   pdb::String setName, int totalX, int totalY, int blockX,
                                                                   int blockY, int label_col_index, string &errMsg,
                                                                   int size = 128, int numPartitions = 1, bool partitionByCol = true);

template void loadMatrixGenericFromFile<pdb::TensorBlock2D<double>>(pdb::PDBClient &pdbClient, std::string filePath, pdb::String dbName,
                                                                    pdb::String setName, int totalX, int totalY, int blockX,
                                                                    int blockY, int label_col_index, string &errMsg,
                                                                    int size = 128, int numPartitions = 1, bool partitionByCol = true);

void load_matrix_from_file(string path, vector<vector<double>> &matrix) {
    if (path.size() == 0) {
        throw runtime_error("Invalid filepath: " + path);
    }

    int totalX, totalY;

    /// 1. Load the data from the file

    // open the input file
    ifstream is(path);
    while (is.peek() == '#' || is.peek() == ' ')
        is.ignore();

    // load the data stats
    is >> totalX;

    while (is.peek() == ',' || is.peek() == ' ')
        is.ignore();

    is >> totalY;

    cout << totalX << ", " << totalY << endl;

    double val;
    for (int i = 0; i < totalX; i++) {
        vector<double> line;
        for (int j = 0; j < totalY; j++) {
            is >> val;
            line.push_back(val);
            while (is.peek() == ',' || is.peek() == ' ')
                is.ignore();
        }
        matrix.push_back(line);
    }
}

bool is_empty_set(pdb::PDBClient &pdbClient, pdb::CatalogClient &catalogClient,
                  string dbName, string setName) {
    if (!catalogClient.setExists(dbName, setName))
        return true;

    auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);
    int count = 0;
    for (auto r : it) {
        count++;
    }

    if (count != 0)
        cout << "[VALIDATE] Set " << setName << " exists!" << endl;

    return count == 0;
}

void print(pdb::PDBClient &pdbClient, string dbName, string setName) {
    auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);
    bool first = true;
    int count = 0;
    for (auto r : it) {
        double *ndata = r->getRawDataHandle()->c_ptr();
        int x = r->getBlockRowIndex();
        int y = r->getBlockColIndex();
        int bx = r->getRowNums();
        int by = r->getColNums();

        if (first) {
            first = false;
            cout << "[STATS] " << setName << endl;
        }

        cout << "[PRINT] " << setName << " : " << x << "," << y
             << "; Block Size: " << bx << "," << by << endl;
        cout << endl;
        count++;
    }
    std::cout << "count = " << count << std::endl;
}

void print_old(pdb::PDBClient &pdbClient, string dbName, string setName) {
    pdb::Map<int, pdb::Map<int, pdb::Handle<FFMatrixBlock>>> data;
    auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

    int max_rows = 0, max_cols = 0;
    int ax = 0, ay = 0, bx = 0, by = 0;

    for (auto r : it) {
        int x = r->getBlockRowIndex();
        int y = r->getBlockColIndex();
        max_rows = max_rows < x ? x : max_rows;
        max_cols = max_cols < y ? y : max_cols;
        ax = r->getTotalRowNums();
        ay = r->getTotalColNums();
        bx = r->getRowNums();
        by = r->getColNums();

        if (data.count(x) != 0 || data[x].count(y) != 0)
            cout << "[WARNING!] Already have " << x << "," << y << " block for "
                 << setName << endl;

        data[x][y] = r;
    }

    long count = 0;

    cout << setName << ": ";
    cout << "Actual dimensions: (" << ax << ", " << ay << "), "
         << "; Block size: (" << bx << ", " << by << ")" << endl;

    for (int i = 0; i <= max_rows; i++) {
        for (int j = 0; j < bx; j++) {
            for (int k = 0; k <= max_cols; k++) {
                for (int l = 0; l < by; l++) {
                    if (data[i][k]->getRawDataHandle() == nullptr)
                        continue;
                    double *ndata = data[i][k]->getRawDataHandle()->c_ptr();
                    int pos = j * by + l;
                    cout << ndata[pos] << ",";
                }
            }
            cout << endl;
        }
        // One row of blocks done
        // cout << endl;
    }
}

void print_stats(pdb::PDBClient &pdbClient, string dbName, string setName) {
    int rows = 0, cols = 0, blocks = 0;
    int totalRows = 0, totalCols = 0;
    int blockRows = 0, blockCols = 0;
    auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

    cout << "[STATS]: " << setName << endl;
    for (auto r : it) {
        // cout << r->getBlockRowIndex() << "," << r->getBlockColIndex() << ";";
        rows = r->getRowNums();
        cols = r->getColNums();
        totalRows = r->getTotalRowNums();
        totalCols = r->getTotalColNums();
        blockRows = max(r->getBlockRowIndex(), blockRows);
        blockCols = max(r->getBlockColIndex(), blockCols);
        blocks++;
    }

    cout << "\n"
         << setName << " (" << (blockRows + 1) << " X " << (blockCols + 1)
         << ") (" << blocks << ") : (" << totalRows << " x " << totalCols
         << "), Each block size: " << rows << " x " << cols << endl;
}

} // namespace ff
