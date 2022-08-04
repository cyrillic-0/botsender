#include <iostream>
#include <cstdio>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cctype>
#include <iomanip>
#include <stdexcept>
#include <curl/curl.h>
#include <json/json.h>
#include <boost/format.hpp>

#include "botsender.h"


// Write HTML response data to std::string
size_t response_to_string(char * ptr, size_t size, size_t nmemb, void * userdata) {
    ((std::string *)userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}

// Example: "привет" -> "%D0%BF%D1%80%D0%B8%D0%B2%D0%B5%D1%82"
std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

BotSender::BotSender() {
    this->curl = curl_easy_init();
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, response_to_string);
    curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &raw_json_data);
}

void BotSender::setBotToken(const std::string & token) {
    this->token = token;
}

void BotSender::setChatID(const int chat_id) {
    this->chat_id = chat_id;
}

std::string BotSender::getBotToken() {
    return this->token;
}

int BotSender::getChatID() {
    return this->chat_id;
}

void BotSender::turnOnVerboseOutput(const std::string & filename) {
    this->verbose = true;
    this->verbose_data = fopen(filename.c_str(), "wb");
    curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(this->curl, CURLOPT_STDERR, verbose_data);
}

void BotSender::turnOffVerboseOutput() {
    curl_easy_setopt(this->curl, CURLOPT_STDERR, stderr);
    curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 0);
    fclose(this->verbose_data);
    this->verbose = false;
}

void BotSender::sendMessage(const std::string & text) {
    std::string url = boost::str(boost::format(url_pattern) % this->token % this->chat_id % url_encode(text));
    std::string error_description;

    // Send request
    curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());
    this->response = curl_easy_perform(this->curl);

    // Check for libcurl errors
    if (this->response != CURLE_OK) {
        throw std::runtime_error(curl_easy_strerror(this->response));
    }

    // Check for Telegram API errors
    this->reader.parse(this->raw_json_data, this->root);
    if (!(this->root["ok"].asBool())) {
        error_description = boost::str(boost::format("HTML Status Code: %1%: %2%") % this->root["error_code"] % this->root["description"].asString());
        throw std::runtime_error(error_description);
    }
}

BotSender::~BotSender() {
    curl_easy_cleanup(this->curl);
    if (verbose) {
        fclose(this->verbose_data);
    }
}