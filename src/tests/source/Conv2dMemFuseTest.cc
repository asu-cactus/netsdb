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
               std::string kernel, std::string res, bool materializeModel) {
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
  if (!pdbClient.executeComputations(errMsg, "conv2d", materializeModel, myWriteSet)) {
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
  if (!pdbClient.executeComputations(errMsg, from, myWriteSet)) {
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
  if (!pdbClient.executeComputations(errMsg, from, myWriteSet)) {
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
  if (!pdbClient.executeComputations(errMsg, "kernel-chunk", myWriteSet)) {
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


void test_conv2d_multiply(pdb::PDBClient &pdbClient, std::string dbName,
                          std::string imageset, std::string kernelset,
                          std::string biasset, bool reloadData, bool materializeModel) {
  std::vector<std::string> allSets{imageset, kernelset, biasset,  "temp_kernel", "temp_kernel1", "temp_kernel2", "kernel_flat", "temp_image",  "temp_image1",   "temp_image2",
                                "result", "result_chunked", "result_chunked1", "result_chunked2"};

  std::vector<std::string> intermediateSets{ "temp_kernel", "temp_kernel1", "temp_kernel2", "kernel_flat",
                                "temp_image",  "temp_image1",   "temp_image2",
                                "result", "result_chunked", "result_chunked1", "result_chunked2"};
  std::vector<std::string> intermediateSets1{"temp_image",  "temp_image1",   "temp_image2",
                                "result", "result_chunked", "result_chunked1", "result_chunked2"};
  std::string errMsg;

  int block_x = 32;
  int block_y = 32;
  int kernel = 1;//kernel shape should be $kernelx$kernel (e.g., 7x7)
  int strides = 1;
  int padding = 0;
  bool block_padding = true;

  int height = 512, width = 512, channels = 3, numOfImages = 100;
  int kHeight = 1, kWidth = 1, kChannels = 3, numOfFilters = 64;
  std::chrono::high_resolution_clock::time_point loadRandomImages_time;
  std::chrono::high_resolution_clock::time_point loadRandomImages_end;
  if (reloadData) {

    test_common::create_database(pdbClient, dbName, allSets, reloadData);

    // Load Image
    std::cout << "Loading images....." << std::endl;
    loadRandomImages_time = std::chrono::high_resolution_clock::now();
    conv2d_memory_fusion::loadRandomImages(width, height, channels, numOfImages, pdbClient, dbName, imageset, 20);
    std::cout << "Image loaded successfully....." << std::endl;
    loadRandomImages_end = std::chrono::high_resolution_clock::now();

    #ifdef PROFILING_CONV2D
          img_conv_flatten::verify_data(pdbClient, dbName, imageset);
    #endif

        // Load Kernel
        std::cout << "Loading kernel....." << std::endl;
        conv2d_memory_fusion::loadRandomImages(kHeight, kWidth, kChannels, numOfFilters, pdbClient, dbName, kernelset, 20);
        std::cout << "Kernel loaded successfully......" << std::endl;
    #ifdef PROFILING_CONV2D
          img_conv_flatten::verify_data(pdbClient, dbName, kernelset);
    #endif
          // std::cout << "Loading bias data..." << std::endl;
          // ff::load_matrix_data(pdbClient, "/home/pavan/netsdb/bias_2048.np", dbName, biasset,
          //     block_x, 1, !block_padding, !block_padding, errMsg);
          // std::cout<< "Successfully loaded matrix data bias-------------------" << std::endl;
    #ifdef PROFILING_CONV2D
          ff::print(pdbClient, dbName, biasset);
    #endif
  } else {
      if (!materializeModel)
          test_common::create_database(pdbClient, dbName, intermediateSets, reloadData);
      else
          test_common::create_database(pdbClient, dbName, intermediateSets1, reloadData);
  }

  // Image ops
  std::cout << "Running image ops..." << std::endl;

  auto image_begin = std::chrono::high_resolution_clock::now();
  img_conv_flatten::img_to_chunks(pdbClient, dbName, imageset, "temp_image",
                                  block_x, block_y, strides, kWidth, padding);

  auto img_to_chunks_time = std::chrono::high_resolution_clock::now();


  #ifdef PROFILING_CONV2D
    img_conv_flatten::verify_chunks(pdbClient, dbName, "temp_image");
  #endif

  auto verify_chunks_time_end = std::chrono::high_resolution_clock::now();

    img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp_image",
                                      "temp_image1", block_x);

  auto chunks_to_blocks_end = std::chrono::high_resolution_clock::now();

  #ifdef PROFILING_CONV2D
    img_conv_flatten::verify_blocks(pdbClient, dbName, "temp_image1");
  #endif

  auto verify_blocks_end = std::chrono::high_resolution_clock::now();


  //X: num_rows per flattened image  (112-7+1)(112-7+1)
  //Y: num_cols per flattened image (7*7*3+1) why +1?
  img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp_image1",
                                     "temp_image2", block_x, block_y,
                                     block_padding, numOfImages * ((width + 2 * padding - kWidth) / strides + 1)  * ((height + 2 * - kHeight) / strides + 1), kWidth * kHeight * kChannels + 1);
 

    auto blocks_to_matrix_end = std::chrono::high_resolution_clock::now();


  //#ifdef PROFILING_CONV2D
  ff::print(pdbClient, dbName, "temp_image2");
  //#endif
  auto image_end = std::chrono::high_resolution_clock::now();

  auto kernel_flat_begin = std::chrono::high_resolution_clock::now();

  if (!materializeModel) {
      // kernel ops
      std::cout << "Running kernel ops..." << std::endl;


      kernel_conv_flatten::kernel_to_chunks(pdbClient, dbName, kernelset,
                                        "temp_kernel", block_x, block_y);

    #ifdef PROFILING_CONV2D
        img_conv_flatten::verify_chunks(pdbClient, dbName, "temp_kernel");
    #endif

    img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "temp_kernel",
                                     "temp_kernel1", block_x);

    #ifdef PROFILING_CONV2D
        img_conv_flatten::verify_blocks(pdbClient, dbName, "temp_kernel1");
    #endif

    img_conv_flatten::blocks_to_matrix(pdbClient, dbName, "temp_kernel1",
                                     "temp_kernel2", block_x, block_y,
                                     block_padding, numOfFilters, kWidth * kHeight * kChannels + 1);

    #ifdef PROFILING_CONV2D
        img_conv_flatten::verify_matrices(pdbClient, dbName, "temp_kernel2");
    #endif

    // // kernel bias join
    // std::cout << "Running kernel bias join..." << std::endl;


    // kernel_conv_flatten::kernel_bias_join(pdbClient, dbName, "temp_kernel2",
    //                                   biasset, "kernel_flat");
  
    #ifdef PROFILING_CONV2D
        ff::print(pdbClient, dbName, "kernel_flat");
    #endif
  }
  auto kernel_flat_end = std::chrono::high_resolution_clock::now();


  // multiply
  std::cout << "Running conv2d op..." << std::endl;

  auto conv2d_begin = std::chrono::high_resolution_clock::now();

  test_common::conv2d_op(pdbClient, dbName, "temp_image2", "kernel_flat", "result", materializeModel);

  auto conv2d_end = std::chrono::high_resolution_clock::now();

  #ifdef PROFILING_CONV2D
    ff::print(pdbClient, dbName, "result");
  #endif



/*  test_common::conv2d_result_to_chunks(pdbClient, 100, dbName, "result", "result_chunked");

#ifdef PROFILING_CONV2D
  img_conv_flatten::verify_chunks(pdbClient, dbName, "result_chunked");
#endif

  img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "result_chunked",
                                     "result_chunked1", 11236);

#ifdef PROFILING_CONV2D
  img_conv_flatten::verify_blocks(pdbClient, dbName, "result_chunked1");
#endif
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

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

  std::cout << "Total Time Duration : "
              << std::chrono::duration_cast<std::chrono::duration<float>>(image_end - image_begin  + kernel_flat_end - kernel_flat_begin + conv2d_end - conv2d_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for image ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(image_end - image_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for kernel flatterning ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(kernel_flat_end - kernel_flat_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for conv2d: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(conv2d_end - conv2d_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for loadRandomImages: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(loadRandomImages_end - loadRandomImages_time).count()
              << " secs." << std::endl;
  std::cout << "Time Duration for img_to_chunks: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(img_to_chunks_time - image_begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for verify_chunks: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(verify_chunks_time_end - img_to_chunks_time).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for chunks_to_blocks: "
          << std::chrono::duration_cast<std::chrono::duration<float>>(chunks_to_blocks_end - verify_chunks_time_end).count()
          << " secs." << std::endl;
  
  std::cout << "Time Duration for verify_blocks: "
          << std::chrono::duration_cast<std::chrono::duration<float>>(verify_blocks_end - chunks_to_blocks_end).count()
          << " secs." << std::endl;

  std::cout << "Time Duration for blocks_to_matrix: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(blocks_to_matrix_end - verify_blocks_end).count()
              << " secs." << std::endl;
  //std::cout << "Time Duration for result gathering: "
  //          << std::chrono::duration_cast<std::chrono::duration<float>>(result_end - result_begin).count()
  //        << " secs." << std::endl;

}

int main(int argc, char *argv[]) {

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("Conv2DclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string errMsg;

  bool reloadData = true;
  if (argc > 1) {
      if (strcmp(argv[1], "N")==0) {
          reloadData = false;
      }
  }

  bool materializeModel = false;
  if (argc > 2) {
      if (strcmp(argv[2], "Y")==0) {
          materializeModel = true;
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

  string dbName = "conv2d";
  string img_set = "img";
  string kernel_set = "kernel";
  test_conv2d_multiply(pdbClient, dbName, img_set, kernel_set, "bias", reloadData, materializeModel);

  return 0;
}

