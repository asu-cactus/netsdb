#ifndef REDDIT_SUB_H
#define REDDIT_SUB_H

//this file describes a subreddit

#include "json.hpp"
#include "PDBVector.h"
#include "PDBString.h"

using namespace pdb;
using json = nlohmann::json;



namespace reddit {

class Sub : public Object {

public:

    ENABLE_DEEP_COPY

    bool accounts_active_is_fuzzed = false;    
    bool allow_discovery = false;    
    bool allow_images = false;  
    bool allow_videogifs = false;   
    bool allow_videos = false;
    String audience_target = "";  
    String banner_img = "";  
    bool can_assign_link_flair = false;   
    bool can_assign_user_flair = false;    
    bool collapse_deleted_comments = false;
    int comment_score_hide_mins = 0; 
    String community_icon = "";    
    double created = 0.0;    
    double created_utc = 0.0;   
    String description = "";
    String description_html = "";    
    String display_name = "";    
    String display_name_prefixed = "";    
    bool emojis_enabled = false;
    bool hide_ads = false;    
    String id = "";    
    String key_color = "";    
    String lang = "";    
    bool link_flair_enabled = false;
    bool link_flair_position = false;    
    String name = "";    
    bool over18 = false;    
    String primary_color = "";    
    String public_description = "";
    String public_description_html = "";    
    bool public_traffic = false;    
    bool quarantine = false;    
    bool show_media = false;    
    bool show_media_preview = false;
    bool spoilers_enabled = false;    
    String submission_type = "";    
    String submit_text = "";    
    String subreddit_type = "";    
    long subscribers = 0;
    String title = "";    
    String url = "";    
    bool user_flair_enabled_in_sr = false;    
    String user_flair_position = "";    
    String user_flair_type = "";
    bool user_has_favorited = false;    
    bool user_is_banned = false;    
    bool user_sr_theme_enabled = false;    
    int videostream_links_count = 0;
    int wls = 0;

    //default constructor
    Sub () {}

    //constructor from a JSON string
    Sub ( std::string jsonStr ) {

        json j = json::parse(jsonStr);

        if (j.find("accounts_active_is_fuzzed")!=j.end()) {
            if (!j.at("accounts_active_is_fuzzed").is_null()) {
                accounts_active_is_fuzzed =
                    j["accounts_active_is_fuzzed"].get<bool>();
            }
        }
        if (j.find("allow_discovery")!=j.end()) {
            if (!j.at("allow_discovery").is_null()) {
                allow_discovery = j["allow_discovery"].get<bool>();
            }
        }
        if (j.find("allow_images")!=j.end()) {
            if (!j.at("allow_images").is_null()) {
                allow_images = j["allow_images"].get<bool>();
            }
        }
        if (j.find("allow_videogifs")!=j.end()) {
            if (!j.at("allow_videogifs").is_null()) {
                allow_images = j["allow_videogifs"].get<bool>();
            }
        }
        if (j.find("allow_videos")!=j.end()) {
            if (!j.at("allow_videos").is_null()) {
                allow_videos = j["allow_videos"].get<bool>();
            }
        }
        if (j.find("can_assign_link_flair")!=j.end()) {
            if (!j.at("can_assign_link_flair").is_null()) {
                can_assign_link_flair = j["can_assign_link_flair"].get<bool>();
            }
        }
        if (j.find("can_assign_user_flair")!=j.end()) {
            if (!j.at("can_assign_user_flair").is_null()) {
                can_assign_user_flair = j["can_assign_user_flair"].get<bool>();
            }
        }
        if (j.find("collapse_deleted_comments")!=j.end()) {
            if (!j.at("collapse_deleted_comments").is_null()) {
                collapse_deleted_comments = 
                    j["collapse_deleted_comments"].get<bool>();
            }
        }
        if (j.find("comment_score_hide_mins")!=j.end()) {
            if (!j.at("comment_score_hide_mins").is_null()) {
                if(j.at("comment_score_hide_mins").type() == 
                        json::value_t::string){
                    comment_score_hide_mins = std::stoi(
                        j["comment_score_hide_mins"].get<std::string>().c_str()
                    );
                } else {
                    comment_score_hide_mins =
                        j["comment_score_hide_mins"].get<int>();
                }
            }
        }
        if (j.find("audience_target")!=j.end()) {
            if (!j.at("audience_target").is_null()) {
                audience_target = j["audience_target"].get<std::string>();
            }
        }
        if (j.find("banner_img")!=j.end()) {
            if (!j.at("banner_img").is_null()) {
                banner_img = j["banner_img"].get<std::string>();
            }
        }
        if (j.find("community_icon")!=j.end()) {
            if (!j.at("community_icon").is_null()) {
                community_icon = j["community_icon"].get<std::string>();
            }
        }
        if (j.find("created")!=j.end()) {
            if (!j.at("created").is_null()) {
                if(j.at("created").type() == json::value_t::string){
                    created = std::stod(
                        j["created"].get<std::string>().c_str()
                    );
                } else {
                    created = j["created"].get<double>();
                }
            }
        }
        if (j.find("created_utc")!=j.end()) {
            if (!j.at("created_utc").is_null()) {
                if(j.at("created_utc").type() == json::value_t::string){
                    created_utc = std::stod(
                        j["created_utc"].get<std::string>().c_str()
                    );
                } else {
                    created_utc = j["created_utc"].get<double>();
                }
            }
        }
        if (j.find("description")!=j.end()) {
            if (!j.at("description").is_null()) {
                description = j["description"].get<std::string>();
            }
        }
        if (j.find("description_html")!=j.end()) {
            if (!j.at("description_html").is_null()) {
                description = j["description_html"].get<std::string>();
            }
        }
        if (j.find("display_name")!=j.end()) {
            if (!j.at("display_name").is_null()) {
                display_name = j["display_name"].get<std::string>();
            }
        }
        if (j.find("display_name_prefixed")!=j.end()) {
            if (!j.at("display_name_prefixed").is_null()) {
                display_name_prefixed =
                    j["display_name_prefixed"].get<std::string>();
            }
        }
        if (j.find("emojis_enabled")!=j.end()) {
            if (!j.at("emojis_enabled").is_null()) {
                emojis_enabled = j["emojis_enabled"].get<bool>();
            }
        }
        if (j.find("hide_ads")!=j.end()) {
            if (!j.at("hide_ads").is_null()) {
                hide_ads = j["hide_ads"].get<bool>();
            }
        }
        if (j.find("id")!=j.end()) {
            if (!j.at("id").is_null()) {
                id = j["id"].get<std::string>();
            }
        }
        if (j.find("key_color")!=j.end()) {
            if (!j.at("key_color").is_null()) {
                key_color = j["key_color"].get<std::string>();
            }
        }
        if (j.find("lang")!=j.end()) {
            if (!j.at("lang").is_null()) {
                lang = j["lang"].get<std::string>();
            }
        }
        if (j.find("link_flair_enabled")!=j.end()) {
            if (!j.at("link_flair_enabled").is_null()) {
                link_flair_enabled = j["link_flair_enabled"].get<bool>();
            }
        }
        if (j.find("name")!=j.end()) {
            if (!j.at("name").is_null()) {
                name = j["name"].get<std::string>();
            }
        }
        if (j.find("over18")!=j.end()) {
            if (!j.at("over18").is_null()) {
                over18 = j["over18"].get<bool>();
            }
        }
        if (j.find("primary_color")!=j.end()) {
            if (!j.at("primary_color").is_null()) {
                primary_color = j["primary_color"].get<std::string>();
            }
        }
        if (j.find("public_description")!=j.end()) {
            if (!j.at("public_description").is_null()) {
                public_description = j["public_description"].get<std::string>();
            }
        }
        if (j.find("public_description_html")!=j.end()) {
            if (!j.at("public_description_html").is_null()) {
                public_description_html =
                    j["public_description_html"].get<std::string>();
            }
        }
        if (j.find("public_traffic")!=j.end()) {
            if (!j.at("public_traffic").is_null()) {
                public_traffic = j["public_traffic"].get<bool>();
            }
        }
        if (j.find("quarantine")!=j.end()) {
            if (!j.at("quarantine").is_null()) {
                quarantine = j["quarantine"].get<bool>();
            }
        }
        if (j.find("show_media")!=j.end()) {
            if (!j.at("show_media").is_null()) {
                show_media = j["show_media"].get<bool>();
            }
        }
        if (j.find("show_media_preview")!=j.end()) {
            if (!j.at("show_media_preview").is_null()) {
                show_media_preview = j["show_media_preview"].get<bool>();
            }
        }
        if (j.find("spoilers_enabled")!=j.end()) {
            if (!j.at("spoilers_enabled").is_null()) {
                spoilers_enabled = j["spoilers_enabled"].get<bool>();
            }
        }
        if (j.find("submission_type")!=j.end()) {
            if (!j.at("submission_type").is_null()) {
                submission_type = j["submission_type"].get<std::string>();
            }
        }
        if (j.find("submit_text")!=j.end()) {
            if (!j.at("submit_text").is_null()) {
                submit_text = j["submit_text"].get<std::string>();
            }
        }
        if (j.find("subscribers")!=j.end()) {
            if (!j.at("subscribers").is_null()) {
                if(j.at("subscribers").type() == json::value_t::string){
                    subscribers = std::stol(
                        j["subscribers"].get<std::string>().c_str()
                    );
                } else {
                    subscribers = j["subscribers"].get<long>();
                }
            }
        }
        if (j.find("title")!=j.end()) {
            if (!j.at("title").is_null()) {
                title = j["title"].get<std::string>();
            }
        }
        if (j.find("url")!=j.end()) {
            if (!j.at("url").is_null()) {
                url = j["url"].get<std::string>();
            }
        }
        if (j.find("user_flair_enabled_in_sr")!=j.end()) {
            if (!j.at("user_flair_enabled_in_sr").is_null()) {
                user_flair_enabled_in_sr =
                    j["user_flair_enabled_in_sr"].get<bool>();
            }
        }
        if (j.find("user_flair_position")!=j.end()) {
            if (!j.at("user_flair_position").is_null()) {
                url = j["user_flair_position"].get<std::string>();
            }
        }
        if (j.find("user_flair_type")!=j.end()) {
            if (!j.at("user_flair_type").is_null()) {
                user_flair_type = j["user_flair_type"].get<std::string>();
            }
        }
        if (j.find("user_has_favorited")!=j.end()) {
            if (!j.at("user_has_favorited").is_null()) {
                user_has_favorited = j["user_has_favorited"].get<bool>();
            }
        }
        if (j.find("user_sr_theme_enabled")!=j.end()) {
            if (!j.at("user_sr_theme_enabled").is_null()) {
                user_sr_theme_enabled = j["user_sr_theme_enabled"].get<bool>();
            }
        }
        if (j.find("videostream_links_count")!=j.end()) {
            if (!j.at("videostream_links_count").is_null()) {
                if(j.at("videostream_links_count").type() ==
                            json::value_t::string){
                    videostream_links_count = std::stoi(
                        j["videostream_links_count"].get<std::string>().c_str()
                    );
                } else {
                    videostream_links_count =
                        j["videostream_links_count"].get<int>();
                }
            }
        }
        if (j.find("wls")!=j.end()) {
            if (!j.at("wls").is_null()) {
                if(j.at("wls").type() == json::value_t::string){
                    wls = std::stoi(j["wls"].get<std::string>().c_str());
                } else {
                    wls = j["wls"].get<int>();
                }
            }
        }
    }

    size_t hash() const override{
        return name.hash();
    }

   std::string toString() override {
        return std::string(name);
    }
    
};


}


#endif
