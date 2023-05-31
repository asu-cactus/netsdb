#include <algorithm>
#include <iostream>
#include <vector>

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

namespace test_common1 {
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
            dbName, s, errMsg, (size_t)8 * (size_t)1024 * (size_t)1024, s)) {
      cout << "Not able to create set " + s + ": " + errMsg;
    } else {
      cout << "Created set " << s << ".\n";
    }
  }
}
}


void pipelined_test_conv2d_multiply(pdb::PDBClient &pdbClient, std::string dbName,
                          std::string imageset, std::string kernelset,
                          std::string biasset, bool reloadData, bool materializeModel) {

  std::vector<std::string> allSets{imageset, kernelset, biasset, "kernel_flat", "image_flat", "result"};

  std::vector<std::string> intermediateSets{"image_flat", "result"};

  std::string errMsg;

  int block_x = 112;
  int block_y = 112;
  int strides = 1;
  int padding = 0;
  bool block_padding = false;

  int height = 112, width = 112, channels = 64, numOfImages = 1;
  int kHeight = 1, kWidth = 1, kChannels = 64, numOfFilters = 64;

  if (reloadData) {

      test_common1::create_database(pdbClient, dbName, allSets, reloadData);

      std::cout << "Loading image data..." << std::endl;
      conv2d_memory_fusion::loadRandomImages(width, height, channels, numOfImages, pdbClient, dbName, imageset, 8);
     
      std::cout << "Loading kernel data..." << std::endl;
      conv2d_memory_fusion::loadRandomImages(kHeight, kWidth, kChannels, numOfFilters, pdbClient, dbName, kernelset, 1);

      std::cout << "Loading bias data..." << std::endl;
      ff::loadMatrix(pdbClient, dbName, biasset, numOfFilters, 1, block_x, 1, !block_padding, !block_padding, errMsg, 1, false);

  } else {

      pdbClient.clearSet(dbName, "image_flat", "FFMatrixBlock", errMsg);
      pdbClient.clearSet(dbName, "result", "FFMatrixBlock", errMsg);

      if (materializeModel == false)
          pdbClient.clearSet(dbName, "kernel_flat", "FFMatrixBlock", errMsg);
  }


  auto begin = std::chrono::high_resolution_clock::now();

  const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 1024};  


  if (!materializeModel) {

      std::cout << "Running kernel ops..." << std::endl;


      pdb::Handle<pdb::Computation> readKernel =
      makeObject<ScanUserSet<conv2d_memory_fusion::Kernel>>(dbName, kernelset);


      pdb::Handle<pdb::Computation> kernel_chunk =
          pdb::makeObject<conv2d_memory_fusion::KernelToChunks>(block_y);
      kernel_chunk->setInput(readKernel);


      pdb::Handle<pdb::Computation> kernel_block =
          pdb::makeObject<conv2d_memory_fusion::ImageChunksToBlock>(block_x);
      kernel_block->setInput(kernel_chunk);


      pdb::Handle<pdb::Computation> kernel_matrix =
         pdb::makeObject<conv2d_memory_fusion::ImageBlockToMatrix>(block_x, block_y,
                                                  block_padding, numOfFilters, kWidth * kHeight * kChannels+1);
      kernel_matrix->setInput(kernel_block);


      // kernel bias join
      std::cout << "Running kernel bias join..." << std::endl;


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

  }

  auto kernel_flat_end = std::chrono::high_resolution_clock::now();


  std::cout << "Running image op..." << std::endl;


  pdb::Handle<pdb::Computation> readA =
      makeObject<ScanUserSet<conv2d_memory_fusion::Image>>(dbName, imageset);

  pdb::Handle<pdb::Computation> chunk =
      pdb::makeObject<conv2d_memory_fusion::ImageToChunks>(block_y, kWidth, strides, padding);

  chunk->setInput(readA);


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
                                                  block_padding, (width - kWidth + 1) * (height - kHeight + 1), kWidth * kHeight * kChannels + 1);
  matrix->setInput(block);


  Handle<Computation> myWriteSet2 =
          makeObject<WriteUserSet<::FFMatrixBlock>>(dbName, "image_flat");
  

  myWriteSet2->setInput(matrix);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "image_ops", myWriteSet2)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  }

  auto image_flat_end = std::chrono::high_resolution_clock::now();


  // multiply
  std::cout << "Running conv2d op..." << std::endl;


  //test_common::conv2d_op(pdbClient, dbName, "temp_image2", "kernel_flat", "result");

  pdb::Handle<pdb::Computation> readD =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, "image_flat");


  pdb::Handle<pdb::Computation> readC =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, "kernel_flat");

  pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
  join->setInput(0, readD);
  join->setInput(1, readC);

  // make the aggregation
  pdb::Handle<pdb::Computation> myAggregation = pdb::makeObject<FFAggMatrix>();
  myAggregation->setInput(join);


  Handle<Computation> myWriteSet3 =
      makeObject<WriteUserSet<FFMatrixBlock>>(dbName, "result");
  myWriteSet3->setInput(myAggregation);

    // run the computation
  if (!pdbClient.executeComputations(errMsg, "conv2d", materializeModel, myWriteSet3)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  auto conv2d_end = std::chrono::high_resolution_clock::now();

/*  std::cout << "gathering results..." << std::endl;

  //test_common::conv2d_result_to_chunks(pdbClient, 100, dbName, "result", "result_chunked");

  pdb::Handle<pdb::Computation> readE =
      makeObject<ScanUserSet<FFMatrixBlock>>(dbName, "result");

  pdb::Handle<pdb::Computation> chonk = pdb::makeObject<conv2d_memory_fusion::ConvResultToChunks>(100);

  chonk->setInput(readE);
  //img_conv_flatten::chunks_to_blocks(pdbClient, dbName, "result_chunked",
                                     //"result_chunked1", 11236);
  
  pdb::Handle<pdb::Computation> chunks_to_blocks =
      pdb::makeObject<conv2d_memory_fusion::ImageChunksToBlock>(11236);
  chunks_to_blocks->setInput(chonk);


  pdb::Handle<pdb::Computation> blocks_to_images = pdb::makeObject<conv2d_memory_fusion::ConvChunksToImage>(106, 106, 64);
  blocks_to_images->setInput(chunks_to_blocks);

  Handle<Computation> myWriteSet1 =
        makeObject<WriteUserSet<conv2d_memory_fusion::Image>>(dbName, "result_chunked2");
  myWriteSet1->setInput(blocks_to_images);

  // run the computation
  if (!pdbClient.executeComputations(errMsg, "result-img", myWriteSet1)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  }

  auto result_end = std::chrono::high_resolution_clock::now();
*/
  std::cout << "Time Duration for kernel ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(kernel_flat_end - begin).count()
              << " secs." << std::endl;

  std::cout << "Time Duration for image ops: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(image_flat_end - kernel_flat_end).count()
              << " secs." << std::endl;


  std::cout << "Time Duration for conv2d: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(conv2d_end - image_flat_end).count()
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

  // test_single_img_conv_flatten(pdbClient, dbName, img_set);
  // test_multiple_img_conv_flatten(pdbClient, dbName, img_set);
  // test_kernel_conv_flatten(pdbClient, dbName, kernel_set);
  pipelined_test_conv2d_multiply(pdbClient, dbName, img_set, kernel_set, "bias", reloadData, materializeModel);

  return 0;
}
