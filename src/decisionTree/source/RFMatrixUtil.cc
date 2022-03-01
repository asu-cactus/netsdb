#include "RFMatrixUtil.h"
#include "RFMatrixBlock.h"
#include "PDBClient.h"
#include <algorithm>
#include <random>
using namespace std;

namespace decisiontree{

void loadMatrix(pdb::PDBClient &pdbClient, pdb::String dbName,
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

  pdb::Handle<pdb::Vector<pdb::Handle<RFMatrixBlock>>> storeMatrix1 =
      pdb::makeObject<pdb::Vector<pdb::Handle<RFMatrixBlock>>>();

  int numXBlocks = ceil(totalX / (float)blockX);
  int numYBlocks = ceil(totalY / (float)blockY);

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

          pdb::Handle<RFMatrixBlock> myData =
              pdb::makeObject<RFMatrixBlock>(i, j, actual_blockX, actual_blockY,
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
      if (!pdbClient.sendData<RFMatrixBlock>(
              pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
      }
    } catch (pdb::NotEnoughSpace &e) {
      if (!pdbClient.sendData<RFMatrixBlock>(
              pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
      }
      std::cout << "Dispatched " << storeMatrix1->size() << " blocks."
                << std::endl;
      pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);
      storeMatrix1 = pdb::makeObject<pdb::Vector<pdb::Handle<RFMatrixBlock>>>();
    }
  }

  cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
       << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
       << blockY << " each" << endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

void load_matrix_from_file(string path, vector<vector<float>> &matrix) {
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

  float val;
  for (int i = 0; i < totalX; i++) {
    vector<float> line;
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

  auto it = pdbClient.getSetIterator<RFMatrixBlock>(dbName, setName);
  int count = 0;
  for (auto r : it) {
    count++;
  }

  if (count != 0)
    cout << "[VALIDATE] Set " << setName << " exists!" << endl;

  return count == 0;
}

void print(pdb::PDBClient &pdbClient, string dbName, string setName) {
  auto it = pdbClient.getSetIterator<RFMatrixBlock>(dbName, setName);
  bool first = true;
  int count = 0;
  for (auto r : it) {
    float *ndata = r->getRawDataHandle()->c_ptr();
    int x = r->getBlockRowIndex();
    int y = r->getBlockColIndex();
    int bx = r->getRowNums();
    int by = r->getColNums();

    if (first) {
      first = false;
      cout << "[STATS] " << setName << " : " << r->getTotalRowNums() << " X " << r->getTotalColNums() << endl;
    }
    count++;
  }
  std::cout << "count = " << count << std::endl;
}

void print_old(pdb::PDBClient &pdbClient, string dbName, string setName) {
  pdb::Map<int, pdb::Map<int, pdb::Handle<RFMatrixBlock>>> data;
  auto it = pdbClient.getSetIterator<RFMatrixBlock>(dbName, setName);

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
          float *ndata = data[i][k]->getRawDataHandle()->c_ptr();
          int pos = j * by + l;
          cout << ndata[pos] << ",";
        }
      }
      cout << endl;
    }
  }
}

void print_stats(pdb::PDBClient &pdbClient, string dbName, string setName) {
  int rows = 0, cols = 0, blocks = 0;
  int totalRows = 0, totalCols = 0;
  int blockRows = 0, blockCols = 0;
  auto it = pdbClient.getSetIterator<RFMatrixBlock>(dbName, setName);

  cout << "[STATS]: " << setName << endl;
  for (auto r : it) {
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

}