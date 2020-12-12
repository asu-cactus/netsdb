#ifndef REDDIT_SUBS_AND_COMMENTS_H
#define REDDIT_SUBS_AND_COMMENTS_H

//this file describes a set of reddit features

#include "json.hpp"
#include "PDBVector.h"
#include "RedditComment.h"
#include "RedditSub.h"

using namespace pdb;

namespace reddit {

class SubsAndComments : public Object {

public:

    ENABLE_DEEP_COPY

    String author;
    long author_created_utc;
    String author_fullname;
    String body;
    long created_utc;
    String id;
    bool no_follows;
    String parent_id;
    String subreddit_name;
    String subreddit_id;
    String description;

    //default constructor
    SubsAndComments () {}

    //constructor from a JSON string
    SubsAndComments ( Handle<Comment> comment, Handle<Sub> subreddit ) {
        author = comment->author;
        author_created_utc = comment->author_created_utc;
        body = comment->body;
        created_utc = comment->created_utc;
        id = comment->id;
        parent_id = comment->parent_id;
        subreddit_name = subreddit->display_name;
        subreddit_id = subreddit->id;
        description = subreddit->description;
    }

    size_t hash() const override{
        return author.hash();
    }
};


}


#endif
