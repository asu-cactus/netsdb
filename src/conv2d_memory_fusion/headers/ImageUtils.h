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

namespace conv2d_memory_fusion {
template <typename M>
void load_imgs_from_file(pdb::PDBClient &pdbClient, std::string path,
                         pdb::String dbName, pdb::String setName);
} // namespace conv2d_memory_fusion
