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
               std::string setName, std::string setName1, int size);

void createSet(pdb::PDBClient &pdbClient, std::string dbName,
               std::string setName, std::string setName1, std::string jobName,
               std::string computationName, std::string lambdaName, int size);



void createDatabase(pdb::PDBClient &pdbClient, std::string dbName);

void setup(pdb::PDBClient &pdbClient, std::string database);

void inference(pdb::PDBClient &pdbClient, std::string database, std::string w1,
               std::string w2, std::string wo, std::string inputs,
               std::string b1, std::string b2, std::string bo,
               std::string output, double dropout_rate, bool enablePartition=false);

void inference_unit(pdb::PDBClient &pdbClient, std::string database, std::string w1,
               std::string wo, std::string inputs,
               std::string b1, std::string bo,
               std::string output, double dropout_rate, bool enablePartition=false);


void inference_unit_log_reg(pdb::PDBClient &pdbClient, std::string database, std::string w, std::string inputs, std::string b, std::string output, double dropout_rate, bool enablePartition=false);


void inference_unit_log_reg1(pdb::PDBClient &pdbClient, std::string database, std::string w, std::string inputs, std::string b, std::string output, bool enablePartition=false);

void inference(pdb::PDBClient &pdbClient, std::string database, std::string w1,
               std::string w2, std::string wo, std::string inputs,
               std::string b1, std::string b2, std::string bo,
               pdb::Handle<pdb::Computation> &output, double dropout_rate, bool enablePartition=false);

void cleanup(pdb::PDBClient &pdblient, std::string database);
} // namespace ff
