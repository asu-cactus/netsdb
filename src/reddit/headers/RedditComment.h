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

    String STRING_DEFAULT = "NULL";
    long LONG_DEFAULT = 0L;
    int INTEGER_DEFAULT = 0;
    bool BOOL_DEFAULT = false;

    int label = INTEGER_DEFAULT;
    int index = INTEGER_DEFAULT;
    bool archived = BOOL_DEFAULT;
    String author = STRING_DEFAULT;
    long author_created_utc = LONG_DEFAULT;
    String author_flair_background_color = STRING_DEFAULT;
    String author_flair_css_class = STRING_DEFAULT;
    Vector<String> author_flair_richtext;
    String author_flair_type = STRING_DEFAULT;
    String author_fullname = STRING_DEFAULT;
    bool author_patreon_flair  = BOOL_DEFAULT;
    String body = STRING_DEFAULT;
    bool can_gild = BOOL_DEFAULT;
    bool can_mod_post = BOOL_DEFAULT;
    bool collapsed = BOOL_DEFAULT;
    String collapsed_reason = STRING_DEFAULT;
    int controversiality = INTEGER_DEFAULT;
    long created_utc = LONG_DEFAULT;
    String distinguished = STRING_DEFAULT;
    String edited = STRING_DEFAULT;
    int gilded = INTEGER_DEFAULT;
    Handle<Map<String, int>> gildings= makeObject<Map<String, int>>();
    String id = STRING_DEFAULT;
    bool is_submitter = BOOL_DEFAULT;
    String link_id = STRING_DEFAULT;
    bool no_follows = BOOL_DEFAULT;
    String parent_id = STRING_DEFAULT;
    String permalink = STRING_DEFAULT;
    String removal_reason = STRING_DEFAULT;
    long retrieved_on = LONG_DEFAULT;
    int score = INTEGER_DEFAULT;
    bool send_replies = BOOL_DEFAULT;
    bool stickied = BOOL_DEFAULT;
    String subreddit = STRING_DEFAULT;
    String subreddit_id = STRING_DEFAULT;
    String subreddit_name_prefixed = STRING_DEFAULT;
    String subreddit_type = STRING_DEFAULT;

    //default constructor
    Comment () {}

    Comment(Comment &other)
        : label(other.label), index(other.index), archived(other.archived),
          author(other.author), author_created_utc(other.author_created_utc),
          author_flair_background_color(other.author_flair_background_color),
          author_flair_css_class(other.author_flair_css_class),
          author_flair_richtext(other.author_flair_richtext),
          author_flair_type(other.author_flair_type),
          author_fullname(other.author_fullname),
          author_patreon_flair(other.author_patreon_flair), body(other.body),
          can_gild(other.can_gild), can_mod_post(other.can_mod_post),
          collapsed(other.collapsed), collapsed_reason(other.collapsed_reason),
          controversiality(other.controversiality),
          created_utc(other.created_utc), distinguished(other.distinguished),
          edited(other.edited), gilded(other.gilded), gildings(other.gildings),
          id(other.id), is_submitter(other.is_submitter),
          link_id(other.link_id), no_follows(other.no_follows),
          parent_id(other.parent_id), permalink(other.permalink),
          removal_reason(other.removal_reason),
          retrieved_on(other.retrieved_on), score(other.score),
          send_replies(other.send_replies), stickied(other.stickied),
          subreddit(other.subreddit), subreddit_id(other.subreddit_id),
          subreddit_name_prefixed(other.subreddit_name_prefixed),
          subreddit_type(other.subreddit_type) {}

    //constructor from a JSON string
    Comment ( std::string jsonStr ) {
        parse_line(jsonStr);
    }

    Comment ( int index, std::string jsonStr ) : index(index) {
        parse_line(jsonStr);
    }

    void parse_line( std::string jsonStr ) {
        json j = json::parse(jsonStr);
        // author_flair_richtext = 
        //     j["author_flair_richtext"].get<std::vector<std::string>>();
        // Each key henceforth is checked for existence
        if (j.find("author")!=j.end()) {
            if (!j.at("author").is_null()) {
                author = j["author"].get<std::string>();
            }
        }
        if (j.find("archived")!=j.end()) {
            if (!j.at("archived").is_null()) {
                archived = j["archived"].get<bool>();
            }
        }
        if (j.find("body")!=j.end()) {
            if (!j.at("body").is_null()) {
                body = j["body"].get<std::string>();
            }
        }
        if (j.find("id")!=j.end()) {
            if (!j.at("id").is_null()) {
                id = j["id"].get<std::string>();
            }
        }
        if (j.find("author_created_utc")!=j.end()) {
            if (!j.at("author_created_utc").is_null()) {
                if(j.at("author_created_utc").type() == json::value_t::string){
                    author_created_utc = std::stol(
                        j["author_created_utc"].get<std::string>().c_str()
                    );
                } else {
                    author_created_utc = j["author_created_utc"].get<long>();
                }
            }
        }
        if (j.find("author_flair_background_color")!=j.end()) {
            if (!j.at("author_flair_background_color").is_null()) {
                author_flair_background_color = 
                    j["author_flair_background_color"].get<std::string>();
            }
        }
        if (j.find("author_flair_css_class")!=j.end()) {
            if (!j.at("author_flair_css_class").is_null()) {
                author_flair_css_class = 
                j["author_flair_css_class"].get<std::string>();
            }
        }
        if (j.find("author_flair_type")!=j.end()) {
            if (!j.at("author_flair_type").is_null()) {
                author_flair_type = j["author_flair_type"].get<std::string>();
            }
        }
        if (j.find("author_fullname")!=j.end()) {
            if (!j.at("author_fullname").is_null()) {
                author_fullname = j["author_fullname"].get<std::string>();
            }
        }
        if (j.find("author_patreon_flair")!=j.end()) {
            if (!j.at("author_patreon_flair").is_null()) {
                author_patreon_flair = j["author_patreon_flair"].get<bool>();
            }
        }
        if (j.find("can_gild")!=j.end()) {
            if (!j.at("can_gild").is_null()) {
                can_gild = j["can_gild"].get<bool>();
            }
        }
        if (j.find("can_mod_post")!=j.end()) {
            if (!j.at("can_mod_post").is_null()) {
                can_mod_post = j["can_mod_post"].get<bool>();
            }
        }
        if (j.find("collapsed")!=j.end()) {
            if (!j.at("collapsed").is_null()) {
                collapsed = j["collapsed"].get<bool>();
            }
        }
        if (j.find("collapsed_reason")!=j.end()) {
            if (!j.at("collapsed_reason").is_null()) {
                collapsed_reason = j["collapsed_reason"].get<std::string>();
            }
        }
        if (j.find("controversiality")!=j.end()) {
            if (!j.at("controversiality").is_null()) {
                if(j.at("controversiality").type() == json::value_t::string){
                    controversiality = std::stoi(
                        j["controversiality"].get<std::string>().c_str()
                    );
                } else {
                    controversiality = j["controversiality"].get<int>();
                }
            }
        }
        if (j.find("created_utc")!=j.end()) {
            if (!j.at("created_utc").is_null()) {
                if(j.at("created_utc").type() == json::value_t::string){
                    created_utc = std::stol(
                        j["created_utc"].get<std::string>().c_str()
                    );
                } else {
                    created_utc = j["created_utc"].get<long>();
                }
            }
        }
        if (j.find("distinguished")!=j.end()) {
            if (!j.at("distinguished").is_null()) {
                distinguished = j["distinguished"].get<std::string>();
            }
        }
        if (j.find("gilded")!=j.end()) {
            if (!j.at("gilded").is_null()) {
                if(j.at("gilded").type() == json::value_t::string){
                    gilded = std::stoi(
                        j["gilded"].get<std::string>().c_str()
                    );
                } else {
                    gilded = j["gilded"].get<int>();
                }
            }
        }
        if (j.find("is_submitter")!=j.end()) {
            if (!j.at("is_submitter").is_null()) {
                is_submitter = j["is_submitter"].get<bool>();
            }
        }
        //edited = j["edited"].get<std::string>();
        //gildings = j["gildings"].get<std::map<std::string, int>>();
        if (j.find("link_id")!=j.end()) {
            if (!j.at("link_id").is_null()) {
                link_id = j["link_id"].get<std::string>();
            }
        }
        if (j.find("no_follows")!=j.end()) {
            if (!j.at("no_follows").is_null()) {
                no_follows = j["no_follows"].get<bool>();
            }
        }
        if (j.find("parent_id")!=j.end()) {
            if (!j.at("parent_id").is_null()) {
                parent_id = j["parent_id"].get<std::string>();
            }
        }
        if (j.find("permalink")!=j.end()) {
            if (!j.at("permalink").is_null()) {
                permalink = j["permalink"].get<std::string>();
            }
        }
        if (j.find("removal_reason")!=j.end()) {
            if (!j.at("removal_reason").is_null()) {
                removal_reason = j["removal_reason"].get<std::string>();
            }
        }
        if (j.find("subreddit")!=j.end()) {
            if (!j.at("subreddit").is_null()) {
                subreddit = j["subreddit"].get<std::string>();
            }
        }
        if (j.find("subreddit_id")!=j.end()) {
            if (!j.at("subreddit_id").is_null()) {
                subreddit_id = j["subreddit_id"].get<std::string>();
            }
        }
        if (j.find("subreddit_name_prefixed")!=j.end()) {
            if (!j.at("subreddit_name_prefixed").is_null()) {
                subreddit_name_prefixed = 
                    j["subreddit_name_prefixed"].get<std::string>();
            }
        }
        if (j.find("subreddit_type")!=j.end()) {
            if (!j.at("subreddit_type").is_null()) {
                subreddit_type = j["subreddit_type"].get<std::string>();
            }
        }
        if (j.find("send_replies")!=j.end()) {
            if (!j.at("send_replies").is_null()) {
                send_replies = j["send_replies"].get<bool>();
            }
        }
        if (j.find("stickied")!=j.end()) {
            if (!j.at("stickied").is_null()) {
                stickied = j["stickied"].get<bool>();
            }
        }
        if (j.find("retrieved_on")!=j.end()) {
            if (!j.at("retrieved_on").is_null()) {
                if(j.at("retrieved_on").type() == json::value_t::string){
                    retrieved_on = std::stol(
                        j["retrieved_on"].get<std::string>().c_str()
                    );
                } else {
                    retrieved_on = j["retrieved_on"].get<long>();
                }
            }
        }
        if (j.find("score")!=j.end()) {
            if (!j.at("score").is_null()) {
                if(j.at("score").type() == json::value_t::string){
                    score = std::stoi(
                        j["score"].get<std::string>().c_str()
                    );
                } else {
                    score = j["score"].get<int>();
                }
            }
        }
    }

    size_t hash() const override{
        //std::cout << "run hash on Comment" << std::endl;
        if (label == 1)
           return author.hash();
        else
           return subreddit_id.hash();
    }

    std::string toString() override {
        return std::string(author)+std::string(",")+std::string(subreddit_id)+std::string(", hash:") + std::to_string(hash());
    }
};


}


#endif
