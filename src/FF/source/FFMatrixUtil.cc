#include "FFMatrixUtil.h"

#include "FFMatrixBlock.h"

#include "PDBClient.h"
#include <algorithm>

using namespace std;

namespace ff {
int load_matrix_data(pdb::PDBClient &pdbClient, string path, pdb::String dbName,
                     pdb::String setName, int blockX, int blockY,
                     bool dont_pad_x, bool dont_pad_y, string &errMsg, bool partitionByCol) {
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

  int numXBlocks = ceil(totalX / (double)blockX);
  int numYBlocks = ceil(totalY / (double)blockY);

  // cout << "Creating " << numXBlocks << " X blocks and " << numYBlocks
  //      << " Y blocks" << endl;

  try {
    for (int i = 0; i < numXBlocks; i++) {
      int actual_blockX =
          dont_pad_x ? min((unsigned long)blockX, matrix.size() - i * blockX)
                     : blockX;

      for (int j = 0; j < numYBlocks; j++) {
        int actual_blockY = dont_pad_y ? min((unsigned long)blockY,
                                             matrix[0].size() - j * blockY)
                                       : blockY;

        pdb::Handle<FFMatrixBlock> myData =
            pdb::makeObject<FFMatrixBlock>(i, j, actual_blockX, actual_blockY,
                                           matrix.size(), matrix[0].size(), partitionByCol);

        for (int ii = 0; ii < actual_blockX; ii++) {
          for (int jj = 0; jj < actual_blockY; jj++) {
            int curX = (i * actual_blockX + ii);
            int curY = (j * actual_blockY + jj);

            if ((dont_pad_x && curX >= totalX) ||
                (dont_pad_y && curY >= totalY)) {
              cout << "Shouldnt be here!" << endl;
              exit(1);
            }

            double data = curX >= totalX || curY >= totalY
                              ? 0 // padding to adjust to block dimensions
                              : matrix[curX][curY];
            (*(myData->getRawDataHandle()))[ii * actual_blockY + jj] = data;
          }
        }

        // cout << "New block: " << total << endl;
        storeMatrix1->push_back(myData);
        total++;
      }
    }
    if (!pdbClient.sendData<FFMatrixBlock>(
            pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
      cout << "Failed to send data to dispatcher server" << endl;
      exit(1);
    }
  } catch (pdb::NotEnoughSpace &e) {
    cout << "Failed to send data to dispatcher server" << endl;
    exit(1);
  }
  cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
       << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
       << blockY << " each" << endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);

  return matrix.size();
}

void loadMatrix(pdb::PDBClient &pdbClient, pdb::String dbName,
                pdb::String setName, int totalX, int totalY, int blockX,
                int blockY, bool dont_pad_x, bool dont_pad_y, string &errMsg, int size, bool partitionByCol) {

  std::random_device rd;

  std::mt19937 e2(rd());

  std::uniform_real_distribution<> distp(0.0001, 0.5);
  std::uniform_real_distribution<> distn(-0.5, -0.0001);

  auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                       std::default_random_engine());

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

          pdb::Handle<FFMatrixBlock> myData = pdb::makeObject<FFMatrixBlock>(
              i, j, actual_blockX, actual_blockY, totalX, totalY, partitionByCol);

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
              double data = curX >= totalX || curY >= totalY
                                ? 0
                                : (bool)gen() ? distn(e2) : distp(e2);
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
      std::cout << "Dispatched " << storeMatrix1->size() << " blocks." << std::endl;
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

bool is_empty_set(pdb::PDBClient &pdbClient, pdb::CatalogClient &catalogClient, string dbName, string setName) {
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

  for (auto r : it) {
    double *ndata = r->getRawDataHandle()->c_ptr();
    int x = r->getBlockRowIndex();
    int y = r->getBlockColIndex();
    int bx = r->getRowNums();
    int by = r->getColNums();

    cout << "[PRINT] " << setName << " : " << x << "," << y << "; Block Size: " << bx << "," << by << endl;
    for (int i = 0; i < bx; i++) {
      for (int j = 0; j < by; j++) {
        cout << ndata[i * by + j] << ",";
      }
      cout << endl;
    }
    cout << endl;
  }
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
      cout << "[WARNING!] Already have " << x << "," << y << " block for " << setName << endl;

    data[x][y] = r;
  }

  long count = 0;

  cout << setName << ": ";
  cout << "Actual dimensions: (" << ax << ", " << ay  << "), " << "; Block size: (" << bx << ", " << by << ")" << endl;

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
       << setName << " (" << (blockRows + 1) << " X " << (blockCols + 1) << ") (" << blocks
       << ") : (" << totalRows << " x " << totalCols
       << "), Each block size: " << rows << " x " << cols << endl;
}

} // namespace ff
