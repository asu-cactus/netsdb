#include "PDBClient.h"
#include <algorithm>

#include "Image.h"
#include "Kernel.h"
#include "Matrix3D.h"

using namespace pdb;
using namespace std;

namespace conv2d_memory_fusion {

template <typename M>
void load_imgs_from_file(PDBClient &pdbClient, string path, String dbName,
                         String setName, int img_count, int channels, int x, int y) {
  static_assert(std::is_base_of<conv2d_memory_fusion::Matrix3D, M>::value,
                "M must be a descendant of conv2d_memory_fusion::Matrix3D");

  string errMsg;
  if (path.size() == 0) {
    throw runtime_error("Invalid filepath: " + path);
  }


  ifstream is;
  is.open(path);
/*  while (is.peek() == '#' || is.peek() == ' ')
    is.ignore();

  is >> img_count;
  std::cout << "img_count=" << img_count << std::endl;
  while (is.peek() == ',' || is.peek() == ' ')
    is.ignore();

  is >> channels;
  std::cout << "channels=" << channels << std::endl;
  while (is.peek() == ',' || is.peek() == ' ')
    is.ignore();

  is >> x;
  std::cout << "x=" << x << std::endl;
  while (is.peek() == ',' || is.peek() == ' ')
    is.ignore();

  is >> y;
  std::cout << "y=" << y << std::endl;
  cout << img_count << ", " << channels << ", " << x << ", " << y << endl;
*/

  //img_count = 100;
  //channels = 3;
  //x = 112;
  //y = 112;
  int total = 0;
  double val;
  pdb::makeObjectAllocatorBlock(2 * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<pdb::Handle<M>>> images =
      pdb::makeObject<pdb::Vector<pdb::Handle<M>>>();

  int im = 0;
  int c = 0;
  int c_created = 0;
  int image_created = 0;
  bool rollback = false;
  pdb::Handle<M> myData = nullptr;
  pdb::Handle<FFMatrixBlock> chan = nullptr;
  while (im < img_count) {
    try {
      if (!rollback)
          myData = pdb::makeObject<M>(im, x, y, channels);

      if (rollback && (c_created > c)) {
          myData->addChannel(chan);
          c++;
      }
       
      if (rollback && (image_created > im)) {
          images->push_back(myData);
          im++;
          myData = pdb::makeObject<M>(im, x, y, channels);
      }
      rollback = false;
      while (c < channels) {
        chan =
            pdb::makeObject<FFMatrixBlock>(0, 0, x, y, x, y);

        // Cant error out after this line, since all memory needed is already
        // allocated
        for (int i = 0; i < x; i++) {
          for (int j = 0; j < y; j++) {
            is >> val;
            (*(chan->getRawDataHandle()))[i * y + j] = val;

            while (is.peek() == ',' || is.peek() == ' ')
              is.ignore();
          }
        }
        c_created++;
        myData->addChannel(chan);
        c++;
      }
      image_created++;
      images->push_back(myData);
      im++;
      c = 0;
      c_created=0;
    } catch (pdb::NotEnoughSpace &e) {
      if (!pdbClient.sendData<M>(pair<string, string>(setName, dbName), images,
                                 errMsg)) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
      }
      std::cout << "Dispatched " << images->size() << " images." << std::endl;
      pdb::makeObjectAllocatorBlock(2 * 1024 * 1024, true);
      images = pdb::makeObject<pdb::Vector<pdb::Handle<M>>>();
      rollback = true;
    }
  }
  is.close();

  if (!pdbClient.sendData<M>(pair<string, string>(setName, dbName), images,
                             errMsg)) {
    cout << "Failed to send data to dispatcher server" << endl;
    exit(1);
  }
  std::cout << "Dispatched " << images->size() << " images." << std::endl;
  std::cout << "Sent total of " << img_count << " images." << std::endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

template void load_imgs_from_file<conv2d_memory_fusion::Image>(PDBClient &pdbClient,
                                                 string path, String dbName,
                                                 String setName, int img_count = 100, int channels=3, int x=112, int y=112);
template void load_imgs_from_file<conv2d_memory_fusion::Kernel>(PDBClient &pdbClient,
                                                  string path, String dbName,
                                                  String setName, int img_count=64, int channels=3, int x=7, int y=7);
} // namespace conv2d_memory_fusion
