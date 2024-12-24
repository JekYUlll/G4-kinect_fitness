#ifndef CONFIGREADER_HPP
#define CONFIGREADER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "log/KFLog.h"

namespace kfc {

    struct Config {
        std::string std_file_path;
        int window_width = 800;
        int window_height = 600;
        int frame_rate = 60;
    };

    inline std::string Trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t");
        auto end = str.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    inline Config ReadConfig(const std::string& configFilePath) {
        Config config;
        std::ifstream configFile(configFilePath);
        if (configFile.is_open()) {
            std::string line;
            while (std::getline(configFile, line)) {
                std::stringstream ss(line);
                std::string key, value;
                if (std::getline(ss, key, '=') && std::getline(ss, value)) {
                    key = Trim(key);
                    value = Trim(value);
                    if (key == "file_path") {
                        config.std_file_path = value;
                        LOG_D("Reading standard action file path: {}", value);
                    }
                    else if (key == "window_width") {
                        config.window_width = std::stoi(value);
                    }
                    else if (key == "window_height") {
                        config.window_height = std::stoi(value);
                    }
                    else if (key == "frame_rate") {
                        config.frame_rate = std::stoi(value);
                    }
                }
            }
            configFile.close();
        }
        else {
            LOG_E("Failed to open config file: {}", configFilePath);
        }
        return config;
    }

    inline void InitConfig(const std::string& configFile) {
        Config config = ReadConfig(configFile);
        if(configFile.empty()){
            LOG_E("Failed to load config file: {}", configFile);
            return;
        } else {
            LOG_I("Successfully loaded config file: {}", configFile);
        }
        
        if (config.std_file_path.empty()) {
            LOG_E("Standard action file path not set");
            return;
        }

        LOG_D("Standard action file path set to: {}", config.std_file_path);

        // Load standard action
        try {
            g_actionTemplate = std::make_unique<ActionTemplate>(config.std_file_path);
            /*if (g_actionTemplate && !g_actionTemplate->getFrames().empty()) {
                LOG_I("Standard action loaded successfully! Total {} frames.", g_actionTemplate->getFrameCount());
            } else {
                LOG_E("Failed to load standard action or frame count is 0");
            }*/
        } catch (const std::exception& e) {
            LOG_E("Exception occurred while loading standard action: {}", e.what());
        }
    }
}

#endif