#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBVector.h"
#include "TensorData.h"

namespace pdb {
class PDBClient;
class CatalogClient;
class String;
} // namespace pdb

namespace conv2d_memory_fusion {
    template <typename M>
    void load_imgs_from_file(pdb::PDBClient &pdbClient, std::string path, 
                            pdb::String dbName, pdb::String setName, int img_count, int channels, int x, int y);

    void loadRandomImages(int width, int height, int channel,
                            int numOfImages, pdb::PDBClient &pdbClient, pdb::String dbName, pdb::String setName, int pageSize);
} // namespace conv2d_memory_fusion
