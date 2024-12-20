#ifndef CONFIGREADER_HPP
#define CONFIGREADER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "log/KFLog.h"
#include "calc/serialize.h"

namespace kf {

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
                        LOG_I("��ȡ��׼�����ļ�·��: {}", value);
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
            LOG_E("�޷��������ļ�: {}", configFilePath);
        }
        return config;
    }

    inline void InitConfig(const std::string& configFile) {
        LOG_I("��ʼ��ȡ�����ļ�: {}", configFile);
        
        Config config = ReadConfig(configFile);
        
        if (config.std_file_path.empty()) {
            LOG_E("��׼�����ļ�·��δ����");
            return;
        }

        LOG_I("��׼�����ļ�·������Ϊ: {}", config.std_file_path);

        // ���ر�׼����
        try {
            g_actionTemplate = std::make_unique<ActionTemplate>(config.std_file_path);
            if (g_actionTemplate && !g_actionTemplate->getFrames().empty()) {
                LOG_I("��׼�������سɹ�! ���� {} ֡���ݡ�", g_actionTemplate->getFrameCount());
            } else {
                LOG_E("��׼��������ʧ�ܻ�֡��Ϊ0");
            }
        } catch (const std::exception& e) {
            LOG_E("���ر�׼����ʱ�����쳣: {}", e.what());
        }
    }
}

#endif