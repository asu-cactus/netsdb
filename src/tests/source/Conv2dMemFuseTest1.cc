#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

#include "FFMatrixBlock.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"

#include "Image.h"
#include "ImageBlock.h"
#include "ImageBlockToMatrix.h"
#include "ImageChunk.h"
#include "ImageChunksToBlock.h"
#include "ImageToChunks.h"
#include "Matrix3D.h"

#include "Kernel.h"
#include "KernelBiasJoin.h"
#include "KernelToChunks.h"
#include "ConvResultToChunks.h"
#include "ConvChunksToImage.h"

#include "FFMatrixUtil.h"
#include "ImageUtils.h"

#include "FFTransposeMult.h"
#include "FFAggMatrix.h"

#include "PDBClient.h"
#include "PDBVector.h"
#include "ScanUserSet.h"
#include "WriteUserSet.h"
using namespace std;

void load_rnd_img(int x, int y, int z, int size, pdb::PDBClient &pdbClient,
                  pdb::String dbName, pdb::String setName) {
  std::string errMsg;
  pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<pdb::Handle<conv2d_memory_fusion::Image>>> storeImage =
      pdb::makeObject<pdb::Vector<pdb::Handle<conv2d_memory_fusion::Image>>>();

  pdb::Handle<conv2d_memory_fusion::Image> myData =
      pdb::makeObject<conv2d_memory_fusion::Image>(0, x, y, z);

  std::random_device rd;

  std::mt19937 e2(rd());

  std::uniform_real_distribution<> distp(0.0001, 0.5);
  std::uniform_real_distribution<> distn(-0.5, -0.0001);

  auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                       std::default_random_engine());

  try {
    for (int c = 0; c < z; c++) {

      pdb::Handle<FFMatrixBlock> chan =
          pdb::makeObject<FFMatrixBlock>(0, 0, x, y, x, y);

      for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
          float data = (bool)gen() ? distn(e2) : distp(e2);
          (*(chan->getRawDataHandle()))[i * y + j] = data;
        }
      }

      myData->addChannel(chan);
    }

    storeImage->push_back(myData);

    if (!pdbClient.sendData<conv2d_memory_fusion::Image>(
            pair<string, string>(setName, dbName), storeImage, errMsg)) {
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

namespace test_common {
void create_database(pdb::PDBClient &pdbClient, std::string dbName,
                     std::vector<std::string> sets, bool whetherToCreateDatabase=true) {
  std::string errMsg;
  std::cout << "Setting up database..." << std::endl;

  if (whetherToCreateDatabase) {
      pdbClient.removeDatabase(dbName, errMsg);
      if (!pdbClient.createDatabase(dbName, errMsg)) {
          cout << "Not able to create database " << dbName << ": " << errMsg << endl;
      } else {
          cout << "Created database " << dbName << endl;
      }
  }
  for (auto &s : sets) {
    pdbClient.removeSet(dbName, s, errMsg);
    if (!pdbClient.createSet<conv2d_memory_fusion::Image>(
            dbName, s, errMsg, (size_t)128 * (size_t)1024 * (size_t)1024, s)) {
      cout << "Not able to create set " + s + ": " + errMsg;
    } else {
      cout << "Created set " << s << ".\n";
    }
  }
}

void conv2d_op(pdb::PDBClient &pdbClient, std::string dbName, std::string image,
               std::string kernel, std::string res) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, image);
  pdb::Handle<pdb::Computation> readB =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, kernel);

  pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
  join->setInput(0, readA);
  join->setInput(1, readB);

  // make the aggregation
  pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
  myAggregation->setInput(join);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<FFMatrixBlock>>(dbName, res);
  myWriteSet->setInput(myAggregation);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "conv2d", true, myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}

void conv2d_result_to_chunks(pdb::PDBClient &pdbClient, int images, std::string dbName, std::string result, std::string to) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, result);

  pdb::Handle<pdb::Computation> chonk = pdb::makeObject<conv2d_memory_fusion::ConvResultToChunks>(images);
  chonk->setInput(readA);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<conv2d_memory_fusion::ImageChunk>>(dbName, to);
  myWriteSet->setInput(chonk);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "result-chonk", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}
} // namespace test_common

namespace img_conv_flatten {
void verify_data(pdb::PDBClient &pdbClient, std::string dbName,
                 std::string setName) {
  std::cout << "Verifying data..." << std::endl;

  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  auto it = pdbClient.getSetIterator<conv2d_memory_fusion::Image>(dbName, setName);

  for (auto r : it) {
    for (int c = 0; c < r->get_num_channels(); c++) {
      pdb::Handle<FFMatrixBlock> chan = r->getMatrixAtIndex(c);
      std::cout << "Channel " << c << std::endl;
      float *data = chan->getValue().rawData->c_ptr();
      for (int i = 0; i < r->x * r->y; i++) {
        std::cout << data[i] << ",";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

void verify_chunks(pdb::PDBClient &pdbClient, std::string dbName,
                   std::string setName) {
  std::cout << "Verifying chunks..." << std::endl;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  auto it = pdbClient.getSetIterator<conv2d_memory_fusion::ImageChunk>(dbName, setName);

  for (auto r : it) {
    r->dump();
  }
}

void verify_blocks(pdb::PDBClient &pdbClient, std::string dbName,
                   std::string setName) {
  std::cout << "Verifying blocks..." << std::endl;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  auto it = pdbClient.getSetIterator<conv2d_memory_fusion::ImageBlock>(dbName, setName);

  for (auto r : it) {
    r->dump();
  }
}

void verify_matrices(pdb::PDBClient &pdbClient, std::string dbName,
                     std::string setName) {
  std::cout << "Verifying matrices..." << std::endl;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    std::cout << "Block " << r->getBlockRowIndex() << ","
              << r->getBlockColIndex() << std::endl;
    float *data = r->getValue().rawData->c_ptr();
    for (int i = 0; i < r->getColNums() * r->getRowNums(); i++) {
      std::cout << data[i] << ",";
    }
    std::cout << std::endl;
  }
}

void img_to_chunks(pdb::PDBClient &pdbClient, std::string dbName,
                   std::string from, std::string to, int block_x, int block_y,
                   int strides, int kernel, int padding) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::Image>>(dbName, from);

  pdb::Handle<pdb::Computation> chunk =
      pdb::makeObject<conv2d_memory_fusion::ImageToChunks>(block_y, kernel, strides, padding);
  chunk->setInput(readA);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<conv2d_memory_fusion::ImageChunk>>(dbName, to);
  myWriteSet->setInput(chunk);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "chunk", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}

void chunks_to_blocks(pdb::PDBClient &pdbClient, std::string dbName,
                      std::string from, std::string to, int block_x) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::ImageChunk>>(dbName, from);

  pdb::Handle<pdb::Computation> block =
      pdb::makeObject<conv2d_memory_fusion::ImageChunksToBlock>(block_x);
  block->setInput(readA);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<conv2d_memory_fusion::ImageBlock>>(dbName, to);
  myWriteSet->setInput(block);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "chunk1", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}

void blocks_to_matrix(pdb::PDBClient &pdbClient, std::string dbName,
                      std::string from, std::string to, int block_x,
                      int block_y, bool block_padding, int X, int Y) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::ImageBlock>>(dbName, from);

  pdb::Handle<pdb::Computation> matrix =
      pdb::makeObject<conv2d_memory_fusion::ImageBlockToMatrix>(block_x, block_y,
                                                  block_padding, X, Y);
  matrix->setInput(readA);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<FFMatrixBlock>>(dbName, to);
  myWriteSet->setInput(matrix);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "chunk2", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }
  //pdbClient.flushData(errMsg);
}
} // namespace img_conv_flatten

namespace kernel_conv_flatten {
void kernel_to_chunks(pdb::PDBClient &pdbClient, std::string dbName,
                      std::string from, std::string to, int block_x,
                      int block_y) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::Kernel>>(dbName, from);

  pdb::Handle<pdb::Computation> chunk =
      pdb::makeObject<conv2d_memory_fusion::KernelToChunks>(block_y);
  chunk->setInput(readA);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<conv2d_memory_fusion::ImageChunk>>(dbName, to);
  myWriteSet->setInput(chunk);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "chunk", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}

void kernel_bias_join(pdb::PDBClient &pdbClient, std::string dbName,
                      std::string a, std::string b, std::string to) {
  std::string errMsg;
  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<::FFMatrixBlock>>(dbName, a);
  pdb::Handle<pdb::Computation> readB =
      makeObject<ScanUserSet<::FFMatrixBlock>>(dbName, b);

  pdb::Handle<pdb::Computation> join =
      pdb::makeObject<conv2d_memory_fusion::KernelBiasJoin>();
  join->setInput(0, readA);
  join->setInput(1, readB);

  Handle<Computation> myWriteSet =
      makeObject<WriteUserSet<::FFMatrixBlock>>(dbName, to);
  myWriteSet->setInput(join);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "kernel_bias_join", myWriteSet)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  //pdbClient.flushData(errMsg);
}
} // namespace kernel_conv_flatten

void test_single_img_conv_flatten(pdb::PDBClient &pdbClient, std::string dbName,
                                  std::string img_set) {
  std::vector<std::string> sets{img_set, "temp", "temp1", "temp2"};

  std::string errMsg;

  test_common::create_database(pdbClient, dbName, sets);

  std::cout << "Loading data..." << std::endl;

  int channels = 2;
  load_rnd_img(6, 6, channels, 64, pdbClient, dbName, img_set);
  img_conv_flatten::verify_data(pdbClient, dbName, img_set);

  int block_x = 20;
  int block_y = 20;
  int kernel = 3;
  int strides = 1;
  int padding = 2;
  bool block_padding = true;

  img_conv_flatten::img_to_chunks(pdbClient, dbName, img_set, "temp", block_x,
                                  block_y, strides, kernel, padding);
  img_conv_flatten::verify_chunks(pdbClient, dbName, "temp");

  img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp", "temp1",
                                     block_x);
  img_conv_flatten::verify_blocks(pdbClient, dbName, "temp1");

  img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp1", "temp2",
                                     block_x, block_y, block_padding, 64,
                                     18 + 1);
  img_conv_flatten::verify_matrices(pdbClient, dbName, "temp2");
}

void test_multiple_img_conv_flatten(pdb::PDBClient &pdbClient,
                                    std::string dbName, std::string img_set) {
  std::vector<std::string> sets{img_set, "temp", "temp1", "temp2"};

  std::string errMsg;

  test_common::create_database(pdbClient, dbName, sets);

  std::cout << "Loading data..." << std::endl;

  int channels = 2;
  conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Image>(
      pdbClient, "/usr/data/conv2d/images_1.np", dbName, img_set, 1, 2, 6, 6);
  img_conv_flatten::verify_data(pdbClient, dbName, img_set);

  int block_x = 20;
  int block_y = 20;
  int kernel = 3;
  int strides = 1;
  int padding = 2;
  bool block_padding = true;

  img_conv_flatten::img_to_chunks(pdbClient, dbName, img_set, "temp", block_x,
                                  block_y, strides, kernel, padding);
  img_conv_flatten::verify_chunks(pdbClient, dbName, "temp");

  img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp", "temp1",
                                     block_x);
  img_conv_flatten::verify_blocks(pdbClient, dbName, "temp1");

  img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp1", "temp2",
                                     block_x, block_y, block_padding, 64,
                                     18 + 1);
  img_conv_flatten::verify_matrices(pdbClient, dbName, "temp2");
}

void test_kernel_conv_flatten(pdb::PDBClient &pdbClient, std::string dbName,
                              std::string setName) {
  std::vector<std::string> sets{setName, "temp", "temp1",
                                "temp2", "bias", "kernel_flat"};
  std::string errMsg;

  test_common::create_database(pdbClient, dbName, sets);

  std::cout << "Loading data..." << std::endl;
  conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Kernel>(
      pdbClient, "/usr/data/conv2d/kernel_3.np", dbName, setName, 3, 2, 3, 3);
  img_conv_flatten::verify_data(pdbClient, dbName, setName);

  int block_x = 10;
  int block_y = 10;
  bool block_padding = true;
  ff::load_matrix_data(pdbClient, "/usr/data/conv2d/bias_3.np", dbName, "bias",
                       block_x, 1, !block_padding, !block_padding, errMsg);
  ff::print(pdbClient, dbName, "bias");

  kernel_conv_flatten::kernel_to_chunks(pdbClient, dbName, setName, "temp",
                                        block_x, block_y);
  img_conv_flatten::verify_chunks(pdbClient, dbName, "temp");

  img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp", "temp1",
                                     block_x);
  img_conv_flatten::verify_blocks(pdbClient, dbName, "temp1");

  img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp1", "temp2",
                                     block_x, block_y, block_padding, 3,
                                     18 + 1);
  img_conv_flatten::verify_matrices(pdbClient, dbName, "temp2");

  kernel_conv_flatten::kernel_bias_join(pdbClient, dbName, "temp2", "bias",
                                        "kernel_flat");
  ff::print(pdbClient, dbName, "kernel_flat");
}

void test_conv2d_multiply(pdb::PDBClient &pdbClient, std::string dbName,
                          std::string imageset, std::string kernelset,
                          std::string biasset, bool reloadData) {
  std::vector<std::string> allSets{kernelset,       biasset,  "temp_kernel", "temp_kernel1", "temp_kernel2", "kernel_flat", "temp_image",  "temp_image1",   "temp_image2",
                                "result", "result_chunked", "result_chunked1", "result_chunked2"};

  std::vector<std::string> intermediateSets{"temp_image",  "temp_image1",   "temp_image2",
                                "result", "result_chunked", "result_chunked1", "result_chunked2"};

  std::string errMsg;

  int block_x = 100;
  int block_y = 100;
  int kernel = 7;//kernel shape should be $kernelx$kernel (e.g., 7x7)
  int strides = 1;
  int padding = 0;
  bool block_padding = true;
  int channels = 3;

  if (reloadData) {

      test_common::create_database(pdbClient, dbName, allSets, reloadData);

      pdbClient.removeSet(dbName, imageset, errMsg);
      if (!pdbClient.createSet<conv2d_memory_fusion::Image>(
            dbName, imageset, errMsg, (size_t)2 * (size_t)1024 * (size_t)1024, imageset)) {
            cout << "Not able to create set " + imageset + ": " + errMsg;
      } else {
            cout << "Created set " << imageset << ".\n";
      }



      std::cout << "Loading image data..." << std::endl;
      conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Image>(
     //     pdbClient, "/home/ubuntu/conv2d/images_1.np", dbName, imageset);
     //     pdbClient, "/home/ubuntu/images_10_3_112_112.np", dbName, imageset);
            pdbClient, "/home/ubuntu/images_100_3_112_112.np", dbName, imageset, 100, 3, 112, 112);
     //     pdbClient, "/home/ubuntu/images_100.np", dbName, imageset);
     //     pdbClient, "/home/ubuntu/images_100_3_224_224.np", dbName, imageset);
     
#ifdef PROFILING_CONV2D
      img_conv_flatten::verify_data(pdbClient, dbName, imageset);
#endif

      std::cout << "Loading kernel data..." << std::endl;
      conv2d_memory_fusion::load_imgs_from_file<conv2d_memory_fusion::Kernel>(
      //    pdbClient, "/home/ubuntu/conv2d/kernel_3.np", dbName, kernelset);
            pdbClient, "/home/ubuntu/conv2d/kernel_64_3_7_7.np", dbName, kernelset, 64, 3, 7, 7);
#ifdef PROFILING_CONV2D
      img_conv_flatten::verify_data(pdbClient, dbName, kernelset);
#endif
      std::cout << "Loading bias data..." << std::endl;
      //ff::load_matrix_data(pdbClient, "/home/ubuntu/conv2d/bias_3.np", dbName, biasset,
      ff::load_matrix_data(pdbClient, "/home/ubuntu/conv2d/bias_64.np", dbName, biasset,
          block_x, 1, !block_padding, !block_padding, errMsg);
#ifdef PROFILING_CONV2D
      ff::print(pdbClient, dbName, biasset);
#endif
  }
  else {

      test_common::create_database(pdbClient, dbName, intermediateSets, reloadData);
  }

  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 1024};

  // Image ops
  std::cout << "Running image ops..." << std::endl;

  auto image_begin = std::chrono::high_resolution_clock::now();



  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::Image>>(dbName, imageset);

  pdb::Handle<pdb::Computation> chunk =
      pdb::makeObject<conv2d_memory_fusion::ImageToChunks>(block_y, kernel, strides, padding);

  chunk->setInput(readA);


  //img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp_image",
                                     //"temp_image1", block_x);

  pdb::Handle<pdb::Computation> block =
      pdb::makeObject<conv2d_memory_fusion::ImageChunksToBlock>(block_x);
  block->setInput(chunk);



  //X: num_rows per flattened image  (112-7+1)(112-7+1)
  //Y: num_cols per flattened image (7*7*3+1) why +1?
  //img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp_image1",
                                     //"temp_image2", block_x, block_y,
                                     //block_padding, 1123600, 147 + 1);

  pdb::Handle<pdb::Computation> matrix =
      pdb::makeObject<conv2d_memory_fusion::ImageBlockToMatrix>(block_x, block_y,
                                                  block_padding, 1123600, 147+1);
  matrix->setInput(block);


  Handle<Computation> myWriteSet2 =
          makeObject<WriteUserSet<::FFMatrixBlock>>(dbName, "temp_image2");

  myWriteSet2->setInput(matrix);

  
  // run the computation
  if (!pdbClient.executeComputations(errMsg, "image_ops", myWriteSet2)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  } 


//#ifdef PROFILING_CONV2D
  pdbClient.flushData(errMsg);
  ff::print(pdbClient, dbName, "temp_image2");
//#endif


  auto image_end = std::chrono::high_resolution_clock::now();
  auto kernel_flat_begin = std::chrono::high_resolution_clock::now();
  if (reloadData) {
  //if (true) {
      // kernel ops
      std::cout << "Running kernel ops..." << std::endl;

      //kernel_conv_flatten::kernel_to_chunks(pdbClient, dbName, kernelset,
                                        //"temp_kernel", block_x, block_y);

      pdb::Handle<pdb::Computation> readKernel =
      makeObject<ScanUserSet<conv2d_memory_fusion::Kernel>>(dbName, kernelset);


      pdb::Handle<pdb::Computation> kernel_chunk =
          pdb::makeObject<conv2d_memory_fusion::KernelToChunks>(block_y);
      kernel_chunk->setInput(readKernel);


      //img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp_kernel",
                                     //"temp_kernel1", block_x);

      pdb::Handle<pdb::Computation> kernel_block =
          pdb::makeObject<conv2d_memory_fusion::ImageChunksToBlock>(block_x);
      kernel_block->setInput(kernel_chunk);


      //img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp_kernel1",
                                     //"temp_kernel2", block_x, block_y,
                                     //block_padding, 64, 147 + 1);

      pdb::Handle<pdb::Computation> kernel_matrix =
         pdb::makeObject<conv2d_memory_fusion::ImageBlockToMatrix>(block_x, block_y,
                                                  block_padding, 64, 147+1);
      kernel_matrix->setInput(kernel_block);


      // kernel bias join
      std::cout << "Running kernel bias join..." << std::endl;


      //kernel_conv_flatten::kernel_bias_join(pdbClient, dbName, "temp_kernel2",
                                     //   biasset, "kernel_flat");
      pdb::Handle<pdb::Computation> readB =
          makeObject<ScanUserSet<::FFMatrixBlock>>(dbName, biasset);

      pdb::Handle<pdb::Computation> join1 =
          pdb::makeObject<conv2d_memory_fusion::KernelBiasJoin>();
      join1->setInput(0, kernel_matrix);
      join1->setInput(1, readB);

      Handle<Computation> myWriteSet =
          makeObject<WriteUserSet<::FFMatrixBlock>>(dbName, "kernel_flat");
      myWriteSet->setInput(join1);

      // run the computation
      if (!pdbClient.executeComputations(errMsg, "kernel_bias_join", myWriteSet)) {
         cout << "Computation failed. Message was: " << errMsg << "\n";
         exit(1);
      }


  
  //#ifdef PROFILING_CONV2D
      ff::print(pdbClient, dbName, "kernel_flat");
  //#endif


  }
  auto kernel_flat_end = std::chrono::high_resolution_clock::now();


  // multiply
  std::cout << "Running conv2d op..." << std::endl;

  auto conv2d_begin = std::chrono::high_resolution_clock::now();

  test_common::conv2d_op(pdbClient, dbName, "temp_image2", "kernel_flat", "result");

  auto conv2d_end = std::chrono::high_resolution_clock::now();

//#ifdef PROFILING_CONV2D
  ff::print(pdbClient, dbName, "result");
//#endif

  // result gather
/*  std::cout << "gathering results..." << std::endl;

  auto result_begin = std::chrono::high_resolution_clock::now();

  test_common::conv2d_result_to_chunks(pdbClient, 100, dbName, "result", "result_chunked");

#ifdef PROFILING_CONV2D
  img_conv_flatten::verify_chunks(pdbClient, dbName, "result_chunked");
#endif

  img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "result_chunked",
                                     "result_chunked1", 11236);

#ifdef PROFILING_CONV2D
  img_conv_flatten::verify_blocks(pdbClient, dbName, "result_chunked1");
#endif
  {

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<ScanUserSet<conv2d_memory_fusion::ImageBlock>>(dbName, "result_chunked1");

    pdb::Handle<pdb::Computation> chonk = pdb::makeObject<conv2d_memory_fusion::ConvChunksToImage>(106, 106, 64);
    chonk->setInput(readA);

    Handle<Computation> myWriteSet =
        makeObject<WriteUserSet<conv2d_memory_fusion::Image>>(dbName, "result_chunked2");
    myWriteSet->setInput(chonk);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "result-img", myWriteSet)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }

  }

#ifdef PROFILING_CONV2D
  pdbClient.flushData(errMsg);

  img_conv_flatten::verify_data(pdbClient, dbName, "result_chunked2");
#endif

  auto result_end = std::chrono::high_resolution_clock::now();

*/

  std::cout << "Time Duration for image ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(image_end - image_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for kernel flatterning ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(kernel_flat_end - kernel_flat_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for conv2d: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(conv2d_end - conv2d_begin).count()
              << " secs." << std::endl;


}

int main(int argc, char *argv[]) {

  

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Conv2DclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string errMsg;
  string dbName = "conv2d";
  string img_set = "img";
  string kernel_set = "kernel";

  bool reloadData = true;
  if (argc > 1) {
      if (strcmp(argv[1], "N")==0) {
          reloadData = false;
      }
  }
  // Load Libraries

  if (reloadData) {

    if (!pdbClient.registerType("libraries/libFFMatrixData.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libFFMatrixData.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libFFMatrixMeta.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libFFMatrixMeta.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libFFMatrixBlock.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libFFMatrixBlock.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseMatrix3D.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseMatrix3D.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImage.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImage.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImageChunk.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImageChunk.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImageToChunks.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImageToChunks.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImageBlock.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImageBlock.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImageChunksToBlock.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImageChunksToBlock.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseImageBlockToMatrix.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseImageBlockToMatrix.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseKernel.so", errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseKernel.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseKernelToChunks.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseKernelToChunks.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseKernelBiasJoin.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseKernelBiasJoin.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libFFTransposeMult.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libFFTransposeMult.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libFFAggMatrix.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libFFAggMatrix.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseConvResultToChunks.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseConvResultToChunks.so"
           << ": " << errMsg << endl;
      exit(-1);
    }

    if (!pdbClient.registerType("libraries/libConv2DMemFuseConvChunksToImage.so",
                                errMsg)) {
      cout << "Couldnt include "
           << "libraries/libConv2DMemFuseConvChunksToImage.so"
           << ": " << errMsg << endl;
      exit(-1);
    }
  }

  // test_single_img_conv_flatten(pdbClient, dbName, img_set);
  // test_multiple_img_conv_flatten(pdbClient, dbName, img_set);
  // test_kernel_conv_flatten(pdbClient, dbName, kernel_set);
  test_conv2d_multiply(pdbClient, dbName, img_set, kernel_set, "bias", reloadData);

  return 0;
}


