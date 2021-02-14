#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace pdb {
class PDBClient;
class CatalogClient;
class String;
} // namespace pdb

namespace ff {
int load_matrix_data(pdb::PDBClient &pdbClient, std::string path,
                     pdb::String dbName, pdb::String setName, int blockX,
                     int blockY, bool dont_pad_x, bool dont_pad_y,
                     std::string &errMsg, bool partitionByCol = true);

void loadMatrix(pdb::PDBClient &pdbClient, pdb::String dbName,
                pdb::String setName, int totalX, int totalY, int blockX,
                int blockY, bool dont_pad_x, bool dont_pad_y,
                std::string &errMsg, int size=128, bool partitionByCol = true);

void load_matrix_from_file(std::string path,
                           std::vector<std::vector<double>> &matrix);

void print_stats(pdb::PDBClient &pdbClient, std::string dbName,
                 std::string setName);

void print(pdb::PDBClient &pdbClient, std::string dbName, std::string setName);

bool is_empty_set(pdb::PDBClient &pdbClient, pdb::CatalogClient &catalogClient, std::string dbName, std::string setName);
} // namespace ff
