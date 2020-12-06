#ifndef COMMENT_FEATURES_H
#define COMMENT_FEATURES_H

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "RedditComment.h"

#include "FFMatrixBlock.h"
#include "FFMatrixData.h"

#include "PDBVector.h"

#include <chrono>
#include <ctime>

using namespace pdb;

using namespace std;
using namespace std::chrono;

namespace reddit {

class CommentFeatures : public Object {
public:
  int index;
  Handle<Vector<double>> features = nullptr;

  ENABLE_DEEP_COPY

  // default constructor
  CommentFeatures() {}

  CommentFeatures(pdb::Handle<Comment> comment) : index(comment->index) {
    features = makeObject<Vector<double>>();
    Vector<double> &feature = (*features);

    feature.push_back((double)comment->archived);

    system_clock::time_point tp_epoch;
    time_point<system_clock, duration<int>> tp_seconds(
        duration<int>(comment->author_created_utc));
    system_clock::time_point tp(tp_seconds);

    time_t tt = system_clock::to_time_t(tp);
    tm utc_tm = *gmtime(&tt);

    feature.push_back((double)utc_tm.tm_mday / 31.0);
    feature.push_back((double)comment->author_patreon_flair);
    feature.push_back((double)comment->can_gild);
    feature.push_back((double)comment->can_mod_post);
    feature.push_back((double)comment->collapsed);
    feature.push_back((double)comment->controversiality);

    time_point<system_clock, duration<int>> tq_seconds(
        duration<int>(comment->created_utc));
    system_clock::time_point tq(tq_seconds);

    time_t ttq = system_clock::to_time_t(tq);
    tm utc_tq = *gmtime(&ttq);

    feature.push_back((double)utc_tq.tm_mday / 31.0);
    feature.push_back((double)comment->gilded);
  }

  int getKey() { return this->index; }

  Vector<double> &getFeature() { return (*features); }

  CommentFeatures &getValue() { return *this; }
};

} // namespace reddit

#endif
