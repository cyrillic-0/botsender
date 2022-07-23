#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cctype>
#include <iomanip>
#include <stdexcept>
#include <curl/curl.h>
#include <json/json.h>
#include <boost/format.hpp>



// Telegram API HTTPS Request Pattern and Terminal Colors
const std::string url_pattern = "https://api.telegram.org/bot%1%/sendMessage?chat_id=-100%2%&text=%3%";
const std::string red = "\033[0;31m";
const std::string nocolor = "\033[0m";


// Write HTML response data to std::string
size_t response_to_string(char * ptr, size_t size, size_t nmemb, void * userdata) {
    ((std::string *)userdata)->append(ptr, size * nmemb);
    return size * nmemb;
}


// For errors
void print_err(const char * text) {
    std::cerr << red << "[ERR] " << text << nocolor << std::endl;
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


class BotInstance {
public:
    BotInstance(const std::string & token, const int chat_id) {
        this->token = token;
        this->chat_id = chat_id;
        this->curl = curl_easy_init();

        curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, response_to_string);
        curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &raw_json_data);
    }

    void sendMessage(const std::string & text) {
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

    ~BotInstance() {
        curl_easy_cleanup(this->curl);
    }

private:
    CURL * curl;
    CURLcode response;
    std::string token;
    int chat_id;
    std::string raw_json_data;
    Json::Reader reader;
    Json::Value root;
};


int main(int argc, char ** argv) {
    try {
	// Open config.json
        Json::Reader reader;
        Json::Value root;
        std::fstream file("config.json");
        if (!file) {
            throw std::runtime_error("Config file(config.json) not found!");
        }

	// Load data
        reader.parse(file, root);
        std::string token = root["token"].asString();
        int chat_id = root["chat_id"].asInt();

	// Create new bot instance
        BotInstance bot(token, chat_id);
        std::string message;

	// Send message from standard input
        std::cout << "Enter message: ";
        getline(std::cin, message);
        bot.sendMessage(std::string(message));
    }
    catch (std::runtime_error & err) {
        print_err(err.what());
        return 1;
    }

    return 0;
}
