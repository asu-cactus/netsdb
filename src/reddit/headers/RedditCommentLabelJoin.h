#ifndef REDDIT_COMMENT_LABEL_JOIN_H
#define REDDIT_COMMENT_LABEL_JOIN_H

#include <JoinComp.h>
#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "FFMatrixBlock.h"
#include "RedditComment.h"

#include <iostream>

using namespace pdb;

namespace reddit {

class CommentLabelJoin : public JoinComp<Comment, ::FFMatrixBlock, Comment> {

public:
  ENABLE_DEEP_COPY

  CommentLabelJoin() = default;

  Lambda<bool> getSelection(Handle<::FFMatrixBlock> in1,
                            Handle<Comment> in2) override {
    // The matrix is a 2 column matrix. First column implies YES and second column implies NO.
    // Each comment is expected 
    return (
            makeLambda(in1, in2, [](Handle<::FFMatrixBlock> &in1, Handle<Comment> &in2) { return in1->rowIndexStart() <= in2->index; }) &&
            // makeLambdaFromMethod(in1, rowIndexStart) <=
            //     makeLambdaFromMember(in2, index) &&
            makeLambda(in1, in2, [](Handle<::FFMatrixBlock> &in1, Handle<Comment> &in2) { return in1->rowIndexEnd() > in2->index; }) &&
            // makeLambdaFromMethod(in1, rowIndexEnd) >
            //     makeLambdaFromMember(in2, index) &&
            // makeLambdaFromMethod(in1, getBlockColIndex) == 0
            makeLambda(in1, [](Handle<::FFMatrixBlock>& in1) { return in1->getBlockColIndex() == 0; })
    );
  }

  Lambda<Handle<Comment>> getProjection(Handle<::FFMatrixBlock> in1,
                                        Handle<Comment> in2) override {
    return makeLambda(
        in1, in2, [](Handle<::FFMatrixBlock> &in1, Handle<Comment> &in2) {
          int pos = (in2->index % in1->getRowNums()) * in1->getColNums();

          double *data = in1->getRawDataHandle()->c_ptr();
          Handle<Comment> tmp = makeObject<Comment>(*in2);
          std::cout << data[pos] << ", " << data[pos + 1] << std::endl;
          tmp->label = data[pos] > data[pos + 1] ? 1 : -1;

          return tmp;
        });
  }
};

} // namespace reddit

#endif