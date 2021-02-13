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


#include <cassert>
#include <chrono>
#include <ctime>


using namespace pdb;

using namespace std;
using namespace std::chrono;

namespace reddit {
int NUM_FEATURES = 100;

void long_to_time(long long_time, tm &time_struct) {
  time_t time = long_time;
  gmtime_r(&time, &time_struct);
}

void push_time_features(tm &utc_tm, Vector<double> &feature) {
  feature.push_back((double)utc_tm.tm_mday / 31.0);
  feature.push_back((double)utc_tm.tm_sec / 60.0);
  feature.push_back((double)utc_tm.tm_min / 59.0);
  feature.push_back((double)utc_tm.tm_hour / 23.0);
  feature.push_back((double)utc_tm.tm_mon / 11.0);
  feature.push_back((double)utc_tm.tm_year / 2021.0);
  feature.push_back((double)utc_tm.tm_wday / 6.0);
  feature.push_back((double)utc_tm.tm_yday / 365.0);
  feature.push_back((double)utc_tm.tm_isdst);
}

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

    tm utc_tm;

    long_to_time(comment->author_created_utc, utc_tm);
    push_time_features(utc_tm, feature);

    long_to_time(comment->created_utc, utc_tm);
    push_time_features(utc_tm, feature);

    long_to_time(comment->retrieved_on, utc_tm);
    push_time_features(utc_tm, feature);

    feature.push_back((double)comment->score / 8000);

    feature.push_back((double)comment->is_submitter);
    feature.push_back((double)comment->no_follows);
    feature.push_back((double)comment->can_mod_post);
    feature.push_back((double)comment->score);
    feature.push_back((double)comment->send_replies);
    feature.push_back((double)comment->stickied);

    feature.push_back((double)comment->gilded);

    feature.push_back((double)comment->author_patreon_flair);
    feature.push_back((double)comment->can_gild);
    feature.push_back((double)comment->can_mod_post);
    feature.push_back((double)comment->collapsed);
    feature.push_back((double)comment->controversiality);
    feature.push_back((double)comment->archived);

    std::random_device rd;
    std::mt19937 e2(rd());
    std::uniform_real_distribution<> distp(1, 3);

    auto gen = std::bind(std::uniform_int_distribution<>(0, 41),
                         std::default_random_engine());


    for (int i = 0; i < NUM_FEATURES - 41; i++) {
      double data = feature[gen()] * distp(e2);
      feature.push_back(data);
    }

    assert(feature.size() == NUM_FEATURES);
  }

  int getKey() { return this->index; }

  Vector<double> &getFeature() { return (*features); }

  CommentFeatures &getValue() { return *this; }
};

} // namespace reddit

#endif
