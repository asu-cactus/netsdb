#ifndef REDDIT_COMMENT_H
#define REDDIT_COMMENT_H

//this file describes a reddit comment

#include "json.hpp"
#include "PDBVector.h"
#include "PDBString.h"

using namespace pdb;
using json = nlohmann::json;

namespace reddit {

class Comment : public Object {

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
    String edited;
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

    //default constructor
    Comment () {}

    //constructor from a JSON string
    Comment ( std::string jsonStr ) {

        json j = json::parse(jsonStr);
        
        archived = j["archived"].get<bool>();
        author = j["author"].get<std::string>();
        if (!j.at("author_created_utc").is_null()) {
            author_created_utc = j["author_created_utc"].get<long>();
        }
        if (!j.at("author_flair_background_color").is_null()) {
            author_flair_background_color = j["author_flair_background_color"].get<std::string>();
        }
        if (!j.at("author_flair_css_class").is_null()) {
            author_flair_css_class = j["author_flair_css_class"].get<std::string>();
        }
        //author_flair_richtext = j["author_flair_richtext"].get<std::vector<std::string>>();
        if (j.find("author_flair_type")!=j.end()) {
            if (!j.at("author_flair_type").is_null()) {
                author_flair_type = j["author_flair_type"].get<std::string>();
            }
        }
        if (!j.at("author_fullname").is_null()) {
            author_fullname = j["author_fullname"].get<std::string>();
        }
        if (j.find("author_patreon_flair")!=j.end()) {
            if (!j.at("author_patreon_flair").is_null()) {
                author_patreon_flair = j["author_patreon_flair"].get<bool>();
            }
        }
        body = j["body"].get<std::string>();
        can_gild = j["can_gild"].get<bool>();
        can_mod_post = j["can_mod_post"].get<bool>();
        collapsed = j["collapsed"].get<bool>();
        if (!j.at("collapsed_reason").is_null()) {
            collapsed_reason = j["collapsed_reason"].get<std::string>();
        }
        controversiality = j["controversiality"].get<int>();
        created_utc = j["created_utc"].get<long>();
        if (!j.at("distinguished").is_null()) {
            distinguished = j["distinguished"].get<std::string>();
        }
        //edited = j["edited"].get<std::string>();
        gilded = j["gilded"].get<int>();
        //gildings = j["gildings"].get<std::map<std::string, int>>();
        id = j["id"].get<std::string>();
        is_submitter = j["is_submitter"].get<bool>();
        link_id = j["link_id"].get<std::string>();
        if (j.find("no_follows")!=j.end()) {
            if (!j.at("no_follows").is_null()) {
                no_follows = j["no_follows"].get<bool>();
            }
        }
        parent_id = j["parent_id"].get<std::string>();
        permalink = j["permalink"].get<std::string>();
        if (!j.at("removal_reason").is_null()) {
            removal_reason = j["removal_reason"].get<std::string>();
        }
        retrieved_on = j["retrieved_on"].get<long>();
        score = j["score"].get<int>();
        send_replies = j["send_replies"].get<bool>();
        stickied = j["stickied"].get<bool>();
        subreddit = j["subreddit"].get<std::string>();
        subreddit_id = j["subreddit_id"].get<std::string>();
        subreddit_name_prefixed = j["subreddit_name_prefixed"].get<std::string>();
        subreddit_type = j["subreddit_type"].get<std::string>();
    }


    size_t hash() const {
        return author.hash();
    }

};


}


#endif
