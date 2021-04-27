#ifndef FF_PAGEINDEXER_H
#define FF_PAGEINDEXER_H

#include "FFMatrixBlock.h"
#include "PDBString.h"
#include "PDBVector.h"
#include "PageIndexer.h"

#include <iostream>
#include <random>
#include <cmath>

using namespace pdb;

class FFPageIndexer : public pdb::PageIndexer<long, FFMatrixBlock> {

  pdb::Vector<double> rawData;
  int dim;
  int num_hashes;

public:
  ENABLE_DEEP_COPY

  FFPageIndexer() {}

  FFPageIndexer(int dim, int num_hashes) : dim(dim), num_hashes(num_hashes) {
    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> distp(0.0001, 0.9999);
    std::uniform_real_distribution<> distn(-0.0001, -0.9999);

    auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                         std::default_random_engine());

    for (int i = 0; i < dim * num_hashes; i++) {
      rawData.push_back((bool)gen() ? distn(e2) : distp(e2));
    }
  }

  void dump() override {
    std::cout << "Called FFINDEXER DUMP: " << std::endl;
    for (int i = 0; i < dim * num_hashes; i++) {
      std::cout << rawData[i] << ", ";
    }
    std::cout << std::endl;
  }

  long hashSet(Vector<Handle<FFMatrixBlock>> &objects) override {
    std::cout << "Called FFINDEXER!" << std::endl;

    double sum = 0;
    long total = 0;
    for (size_t i = 0; i < objects.size(); i++) {
      sum += objects[i]->getSum();
      total += (objects[i]->getRowNums() * objects[i]->getColNums());
    }

    return std::ceil(sum);
  }
};

#endif
