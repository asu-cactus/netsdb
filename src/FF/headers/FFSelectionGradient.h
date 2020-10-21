#ifndef FF_SELECTION_GRADIENT_H
#define FF_SELECTION_GRADIENT_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"

using namespace pdb;

class FFSelectionGradient : public SelectionComp<FFMatrixBlock, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  // how many rows does the whole matrix have
  int32_t num_row_blocks{};
  int32_t num_col_blocks{};

  // the labels we use to calculate the gradient
  pdb::Vector<int32_t> labels_meta_pos;
  pdb::Vector<int32_t> labels_meta_cnt;
  pdb::Vector<int32_t> labels_data_col;
  pdb::Vector<int32_t> labels_data_row;

  FFSelectionGradient() = default;
  FFSelectionGradient(
      int32_t num_row_blocks, int32_t num_col_blocks,
      const std::vector<std::pair<int32_t, int32_t>> &labels_meta,
      const std::vector<std::pair<int32_t, int32_t>> &labels_data)
      : num_row_blocks(num_row_blocks), num_col_blocks(num_col_blocks),
        labels_meta_pos(labels_meta.size(), labels_meta.size()),
        labels_meta_cnt(labels_meta.size(), labels_meta.size()),
        labels_data_col(labels_data.size(), labels_data.size()),
        labels_data_row(labels_data.size(), labels_data.size()) {

    // copy the meta
    for (int i = 0; i < labels_meta.size(); ++i) {
      this->labels_meta_pos[i] = labels_meta[i].first;
      this->labels_meta_cnt[i] = labels_meta[i].second;
    }

    // copy the data
    for (int i = 0; i < labels_data.size(); ++i) {
      this->labels_data_col[i] = labels_data[i].first;
      this->labels_data_row[i] = labels_data[i].second;
    }
  }

  Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe,
                      [](Handle<FFMatrixBlock> &checkMe) { return true; });
  }

  Lambda<Handle<FFMatrixBlock>>
  getProjection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [&](Handle<FFMatrixBlock> &checkMe) {
      // make an output
      Handle<FFMatrixBlock> out = makeObject<FFMatrixBlock>(
          checkMe->getBlockRowIndex(), checkMe->getBlockColIndex(),
          checkMe->getRowNums(), checkMe->getColNums());

      // do the sigmoid function
      auto data = checkMe->getValue().rawData->c_ptr();
      auto outData = out->getValue().rawData->c_ptr();
      for (int32_t i = 0; i < checkMe->getRowNums() * checkMe->getColNums();
           i++) {
        outData[i] = 1 / (1 + exp(-data[i]));
      }

      // do the stuff we need to
      auto &meta_pos =
          labels_meta_pos[checkMe->getBlockRowIndex() * num_col_blocks +
                          checkMe->getBlockColIndex()];
      auto &meta_cnt =
          labels_meta_cnt[checkMe->getBlockRowIndex() * num_col_blocks +
                          checkMe->getBlockColIndex()];
      for (int idx = 0; idx < meta_cnt; idx++) {
        auto pos_row = labels_data_row[meta_pos + idx];
        auto pos_col = labels_data_col[meta_pos + idx];
        outData[pos_row * checkMe->getColNums() + pos_col] -= 1;
      }

      return out;
    });
  }
};

#endif
