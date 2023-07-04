#ifndef REDDIT_COMMENT_LABEL_JOIN_H
#define REDDIT_COMMENT_LABEL_JOIN_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include <JoinComp.h>

#include "CommentsChunk.h"
#include "FFMatrixBlock.h"

#include <iostream>

using namespace pdb;

namespace reddit {

class CommentLabelJoin
    : public JoinComp<CommentsChunk, ::FFMatrixBlock, CommentsChunk> {

public:
  ENABLE_DEEP_COPY

  CommentLabelJoin() = default;

  Lambda<bool> getSelection(Handle<::FFMatrixBlock> in1,
                            Handle<CommentsChunk> in2) override {
    return makeLambdaFromMethod(in1, getBlockRowIndex) ==
           makeLambdaFromMethod(in2, getChunkIndex);
  }

  Lambda<Handle<CommentsChunk>>
  getProjection(Handle<::FFMatrixBlock> in1,
                Handle<CommentsChunk> in2) override {
    return makeLambda(
        in1, in2, [](Handle<::FFMatrixBlock> &in1, Handle<CommentsChunk> &in2) {
          float *data = in1->getRawDataHandle()->c_ptr();
          Handle<CommentsChunk> tmp = makeObject<CommentsChunk>(*in2);

          Map<int, Comment> &rhs = tmp->getChunk();
          auto iter = rhs.begin();
          while (iter != rhs.end()) {
            int myKey = (*iter).key;
            int pos = (myKey % in1->getRowNums()) * in1->getColNums();
            if (pos >= in1->getRowNums() * in1->getColNums()) {
              std::cout << "EXCEEDED BOUNDS! FAILED!!" << std::endl;
              exit(1);
            }
            (*iter).value.label = data[pos] > data[pos + 1] ? 1 : -1;
            ++iter;
          }

          return tmp;
        });
  }
};

} // namespace reddit

#endif
