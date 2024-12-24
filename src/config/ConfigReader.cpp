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

bool InitConfig(const std::string& configPath) {
    auto& config = Config::getInstance();
    std::map<std::string, std::string> configMap;
    
    if (!ReadConfig(configPath, configMap)) {
        LOG_E("Failed to read config file");
        return false;
    }
    
    for (const auto& [key, value] : configMap) {
        try {
            switch (hash_str(key.c_str(), key.length())) {
            case "window.width"_hash:
                config.windowWidth = std::stoi(value);
                break;
            case "window.height"_hash:
                config.windowHeight = std::stoi(value);
                break;
            case "fps.display"_hash:
                config.displayFPS = std::stoi(value);
                break;
            case "fps.record"_hash:
                config.recordFPS = std::stoi(value);
                break;
            case "fps.compare"_hash:
                config.compareFPS = std::stoi(value);
                break;
            case "action.standardPath"_hash:
                config.standardActionPath = value;
                break;
            case "action.bufferSize"_hash:
                config.actionBufferSize = std::stoi(value);
                break;
            case "similarity.speedWeight"_hash:
                config.speedWeight = std::stof(value);
                break;
            case "similarity.minSpeedRatio"_hash:
                config.minSpeedRatio = std::stof(value);
                break;
            case "similarity.maxSpeedRatio"_hash:
                config.maxSpeedRatio = std::stof(value);
                break;
            case "similarity.minSpeedPenalty"_hash:
                config.minSpeedPenalty = std::stof(value);
                break;
            case "similarity.dtwBandwidthRatio"_hash:
                config.dtwBandwidthRatio = std::stof(value);
                break;
            case "similarity.threshold"_hash:
                config.similarityThreshold = std::stof(value);
                break;
            default:
                LOG_W("Unknown config key: {}", key);
                break;
            }
        } catch (const std::exception& e) {
            LOG_E("Error parsing config value for {}: {}", key, e.what());
        }
    }
    
    // 验证配置值的合法性
    config.windowWidth = std::max(640, std::min(1920, config.windowWidth));
    config.windowHeight = std::max(480, std::min(1080, config.windowHeight));
    config.displayFPS = std::max(30, std::min(120, config.displayFPS));
    config.recordFPS = std::max(10, std::min(60, config.recordFPS));
    config.compareFPS = std::max(1, std::min(30, config.compareFPS));
    config.actionBufferSize = std::max(20, std::min(300, config.actionBufferSize));
    
    config.speedWeight = std::max(0.0f, std::min(1.0f, config.speedWeight));
    config.minSpeedRatio = std::max(0.1f, config.minSpeedRatio);
    config.maxSpeedRatio = std::max(config.minSpeedRatio + 0.1f, config.maxSpeedRatio);
    config.minSpeedPenalty = std::max(0.0f, std::min(1.0f, config.minSpeedPenalty));
    config.dtwBandwidthRatio = std::max(0.1f, std::min(1.0f, config.dtwBandwidthRatio));
    config.similarityThreshold = std::max(0.0f, std::min(1.0f, config.similarityThreshold));
    
    LOG_I("Configuration loaded:\n"
          "  Window: {}x{}\n"
          "  FPS: display={}, record={}, compare={}\n"
          "  Standard action: {}\n"
          "  Similarity: weight={:.2f}, speedRatio={:.2f}-{:.2f}, penalty={:.2f}, "
          "bandWidth={:.2f}, threshold={:.2f}",
          config.windowWidth, config.windowHeight,
          config.displayFPS, config.recordFPS, config.compareFPS,
          config.standardActionPath,
          config.speedWeight, config.minSpeedRatio, config.maxSpeedRatio,
          config.minSpeedPenalty, config.dtwBandwidthRatio, config.similarityThreshold);

    try {
        g_actionTemplate = std::make_unique<ActionTemplate>(config.standardActionPath);
    }
    catch (const std::exception& e) {
        LOG_E("Load standard action: {}", e.what());
    }
    
    return true;
}

}

