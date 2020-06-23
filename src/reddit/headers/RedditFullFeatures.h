#ifndef REDDIT_FULLFEATURES_H
#define REDDIT_FULLFEATURES_H

//this file describes a set of reddit features

#include "json.hpp"
#include "PDBVector.h"
#include "RedditComment.h"
#include "RedditAuthor.h"
#include "RedditSub.h"

using namespace pdb;

namespace reddit {

class FullFeatures : public Object {

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
    long field02;
    long field03;
    int field04;
    int field05;
    String description;

    //default constructor
    FullFeatures () {}

    //constructor from a JSON string
    FullFeatures ( Handle<Comment> comment, Handle<Author> account, Handle<Sub> subreddit ) {
        author = comment->author;
        author_created_utc = comment->author_created_utc;
        body = comment->body;
        created_utc = comment->created_utc;
        id = comment->id;
        parent_id = comment->parent_id;
        subreddit_name = subreddit->display_name;
        subreddit_id = subreddit->id;
        field02 = account->field02;
        field03 = account->field03;
        field04 = account->field04;
        field05 = account->field05;
        description = subreddit->description;
    }

    size_t hash() const {
        return author.hash();
    }
};


}


#endif
