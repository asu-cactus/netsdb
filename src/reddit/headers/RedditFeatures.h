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

    bool archived;
    String author;
    long author_created_utc;
    String author_flair_background_color;
    String author_flair_css_class;
    Vector<String> author_flair_richtext;
    String author_flair_type;
    String author_fullname;
    bool author_patreon_flair;
    String body;
    bool can_gild;
    bool can_mod_post;
    bool collapsed;
    String collapsed_reason;
    int controversiality;
    long created_utc;
    String distinguished;
    bool edited;
    int gilded;
    Handle<Map<String, int>> gildings;
    String id;
    bool is_submitter;
    String link_id;
    bool no_follows;
    String parent_id;
    String permalink;
    String removal_reason;
    long retrieved_on;
    int score;
    bool send_replies;
    bool stickied;
    String subreddit;
    String subreddit_id;
    String subreddit_name_prefixed;
    String subreddit_type;
    long field02;
    long field03;
    int field04;
    int field05;

    //default constructor
    Features () {}

    //constructor from a JSON string
    Features ( Handle<Comment> comment, Handle<Author> account ) {
        archived = comment->archived;
        author = comment->author;
        author_created_utc = comment->author_created_utc;
        author_flair_background_color = comment->author_flair_background_color;
        author_flair_css_class = comment->author_flair_css_class;
        //author_flair_richtext = j["author_flair_richtext"].get<std::vector<std::string>>();
        author_flair_type = comment->author_flair_type;
        author_fullname = comment->author_fullname;
        author_patreon_flair = comment->author_patreon_flair;
        body = comment->body;
        can_gild = comment->can_gild;
        can_mod_post = comment->can_mod_post;
        collapsed = comment->collapsed;
        collapsed_reason = comment->collapsed_reason;
        controversiality = comment->controversiality;
        created_utc = comment->created_utc;
        distinguished = comment->distinguished;
        edited = comment->edited;
        gilded = comment->gilded;
        //gildings = j["gildings"].get<std::map<std::string, int>>();
        id = comment->id;
        is_submitter = comment->is_submitter;
        link_id = comment->link_id;
        no_follows = comment->no_follows;
        parent_id = comment->parent_id;
        permalink = comment->permalink;
        removal_reason = comment->removal_reason;
        retrieved_on = comment->retrieved_on;
        score = comment->score;
        send_replies = comment->send_replies;
        stickied = comment->stickied;
        subreddit = comment->subreddit;
        subreddit_id = comment->subreddit_id;
        subreddit_name_prefixed = comment->subreddit_name_prefixed;
        subreddit_type = comment->subreddit_type;
        field02 = account->field02;
        field03 = account->field03;
        field04 = account->field04;
        field05 = account->field05;
    }


};


}


#endif
