#include "config/ConfigReader.h"

namespace kfc {

    bool ReadConfig(const std::string& filename, std::map<std::string, std::string>& config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_E("Failed to open config file: {}", filename);
        return false;
    }

    std::string line;
    std::string currentSection;
    while (std::getline(file, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;
        
        // 去除首尾空格
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        // 处理TOML节
        if (line[0] == '[' && line[line.length()-1] == ']') {
            currentSection = line.substr(1, line.length()-2);
            continue;
        }
        
        std::istringstream iss(line);
        std::string key, value;
        
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // 去除首尾空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // 如果在某个节中，添加节名作为前缀
            if (!currentSection.empty()) {
                key = currentSection + "." + key;
            }
            
            // 移除注释
            size_t commentPos = value.find('#');
            if (commentPos != std::string::npos) {
                value = value.substr(0, commentPos);
                value.erase(value.find_last_not_of(" \t") + 1);
            }
            
            // 移除引号（如果存在）
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            config[key] = value;
        }
    }
    
    return true;
}

bool InitConfig() {
    std::map<std::string, std::string> configMap;
    Config& config = Config::getInstance();
    
    // 设置默认值
    config.dataDir = KF_DATA_DIR;
    config.configFile = KF_CONFIG_FILE;
    
    if (!ReadConfig(config.configFile, configMap)) {
        LOG_W("Using default configuration");
        return false;
    }
    
    // 读取配置值
    for (const auto& pair : configMap) {
        const auto& key = pair.first;
        const auto& value = pair.second;
        
        try {
            if (key == "window.width") {
                config.windowWidth = std::stoi(value);
            } else if (key == "window.height") {
                config.windowHeight = std::stoi(value);
            } else if (key == "window.frameRate") {
                config.frameRate = std::stoi(value);
            } else if (key == "record.interval") {
                config.recordInterval = static_cast<INT64>(std::stoll(value)) * 10000; // 转换为微秒
            } else if (key == "action.standardPath") {
                config.standardActionPath = value;
            } else if (key == "similarity.speedWeight") {
                config.speedWeight = std::stof(value);
            } else if (key == "similarity.minSpeedRatio") {
                config.minSpeedRatio = std::stof(value);
            } else if (key == "similarity.maxSpeedRatio") {
                config.maxSpeedRatio = std::stof(value);
            } else if (key == "similarity.minSpeedPenalty") {
                config.minSpeedPenalty = std::stof(value);
            } else if (key == "similarity.dtwBandwidthRatio") {
                config.dtwBandwidthRatio = std::stof(value);
            } else if (key == "similarity.threshold") {
                config.similarityThreshold = std::stof(value);
            }
        } catch (const std::exception& e) {
            LOG_E("Error parsing config value for {}: {}", key, e.what());
        }
    }
    
    // 验证配置值的合法性
    config.windowWidth = std::max(640, std::min(1920, config.windowWidth));
    config.windowHeight = std::max(480, std::min(1080, config.windowHeight));
    config.frameRate = std::max(15, std::min(60, config.frameRate));
    config.recordInterval = std::max<INT64>(100 * 10000, std::min<INT64>(1000 * 10000, config.recordInterval));
    
    config.speedWeight = std::max(0.0f, std::min(1.0f, config.speedWeight));
    config.minSpeedRatio = std::max(0.1f, config.minSpeedRatio);
    config.maxSpeedRatio = std::max(config.minSpeedRatio + 0.1f, config.maxSpeedRatio);
    config.minSpeedPenalty = std::max(0.0f, std::min(1.0f, config.minSpeedPenalty));
    config.dtwBandwidthRatio = std::max(0.1f, std::min(1.0f, config.dtwBandwidthRatio));
    config.similarityThreshold = std::max(0.0f, std::min(1.0f, config.similarityThreshold));
    
    LOG_I("Configuration loaded:\n"
          "  Window: {}x{} @{}fps\n"
          "  Record interval: {}ms\n"
          "  Standard action: {}\n"
          "  Similarity: weight={:.2f}, speedRatio={:.2f}-{:.2f}, penalty={:.2f}, "
          "bandWidth={:.2f}, threshold={:.2f}",
          config.windowWidth, config.windowHeight, config.frameRate,
          config.recordInterval / 10000,
          config.standardActionPath,
          config.speedWeight, config.minSpeedRatio, config.maxSpeedRatio,
          config.minSpeedPenalty, config.dtwBandwidthRatio, config.similarityThreshold);
    
    return true;
}

}

