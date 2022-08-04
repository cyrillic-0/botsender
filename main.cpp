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

using namespace std;


// For errors
void print_err(const char * text) {
    cerr << red << "[ERR] " << text << nocolor << endl;
}

int main(int argc, char ** argv) {
    try {
        bool verbose = false;

	// Open config.json
        Json::Reader reader;
        Json::Value root;
        fstream file("config.json");
        if (!file) {
            throw runtime_error("Config file(config.json) not found!");
        }

	// Load data
        reader.parse(file, root);
        string token = root["token"].asString();
        int chat_id = root["chat_id"].asInt();

    // Get flags
        for (int i = 0; i < argc; i++) {
            if (!strcmp(argv[i], "-h")) {
                cout << help << endl;
                return 1;
            }
            else if (!strcmp(argv[i], "-ver")) {
                cout << version << endl;
                return 1;
            }
            else if (!strcmp(argv[i], "-v")) {
                verbose = true;
            }
        }

	// Create new bot instance
        BotSender bot;
        bot.setBotToken(token);
        bot.setChatID(chat_id);
        if (verbose) {
            bot.turnOnVerboseOutput(root["verbose_data"].asString());
        }
        string message;

	// Send message from standard input
        cout << "Enter message: ";
        getline(cin, message);
        bot.sendMessage(string(message));
    }
    catch (runtime_error & err) {
        print_err(err.what());
        return 1;
    }

    return 0;
}
