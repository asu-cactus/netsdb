#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace pdb {
class PDBClient;
template <typename> class Handle;
class Computation;
} // namespace pdb

namespace ff {
void loadLibrary(pdb::PDBClient &pdbClient, std::string path);

void createSet(pdb::PDBClient &pdbClient, std::string dbName,
               std::string setName, std::string setName1);



void createDatabase(pdb::PDBClient &pdbClient, std::string dbName);

void setup(pdb::PDBClient &pdbClient, std::string database);

void inference(pdb::PDBClient &pdbClient, std::string database, std::string w1,
               std::string w2, std::string wo, std::string inputs,
               std::string b1, std::string b2, std::string bo,
               std::string output, double dropout_rate);

void inference(pdb::PDBClient &pdbClient, std::string database, std::string w1,
               std::string w2, std::string wo, std::string inputs,
               std::string b1, std::string b2, std::string bo,
               pdb::Handle<pdb::Computation> &output, double dropout_rate);
} // namespace ff
