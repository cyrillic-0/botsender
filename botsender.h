#pragma once

#include <string>
#include <cstdio>
#include <curl/curl.h>
#include <json/json.h>

static const std::string url_pattern = "https://api.telegram.org/bot%1%/sendMessage?chat_id=-100%2%&text=%3%";
static const std::string red = "\033[0;31m";
static const std::string nocolor = "\033[0m";
static const std::string help = "Usage: botsender <flags>\n\n-v - verbose output\n\n-h - help\n\n-v - version\n\n";
static const std::string version = "Version: v1.2";

size_t response_to_string(char * ptr, size_t size, size_t nmemb, void * userdata);

std::string url_encode(const std::string &value);


class BotSender {
public:
    BotSender();
    void setBotToken(const std::string & token);
    void setChatID(const int chat_id);
    std::string getBotToken();
    int getChatID();
    void turnOnVerboseOutput(const std::string & filename);
    void turnOffVerboseOutput();
    void sendMessage(const std::string & text);
    ~BotSender();

private:
    CURL * curl;
    CURLcode response;
    std::string token;
    int chat_id;
    std::string raw_json_data;
    Json::Reader reader;
    Json::Value root;
    bool verbose;
    FILE * verbose_data;
};