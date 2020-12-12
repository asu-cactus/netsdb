#ifndef REDDIT_FEATURES_H
#define REDDIT_FEATURES_H

//this file describes a set of reddit features

#include "json.hpp"
#include "PDBVector.h"
#include "RedditComment.h"
#include "RedditAuthor.h"


using namespace pdb;

namespace reddit {

class Features : public Object {

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
    String subreddit;
    String subreddit_id;
    long field02;
    long field03;
    int field04;
    int field05;

    //default constructor
    Features () {}

    //constructor from a JSON string
    Features ( Handle<Comment> comment, Handle<Author> account ) {
        author = comment->author;
        author_created_utc = comment->author_created_utc;
        body = comment->body;
        created_utc = comment->created_utc;
        id = comment->id;
        parent_id = comment->parent_id;
        subreddit = comment->subreddit;
        subreddit_id = comment->subreddit_id;
        field02 = account->field02;
        field03 = account->field03;
        field04 = account->field04;
        field05 = account->field05;
    }

    size_t hash() const override{
        return author.hash();
    }

    std::string toString() override{
        return std::string(author.c_str())+":"+std::string(subreddit_id.c_str());
    }

};


}


#endif
