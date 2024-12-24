#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#define NOMINMAX

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include "calc/serialize.h"
#include "KFcommon.h"

namespace kfc {

struct Config {
    // 基础配置
    std::string dataDir;            // 数据目录
    std::string configFile;         // 配置文件路径
    std::string standardActionPath; // 标准动作文件路径
    int windowWidth;               // 窗口宽度
    int windowHeight;              // 窗口高度
    int frameRate;                 // 帧率
    INT64 recordInterval;          // 记录间隔(微秒)
    
    // 相似度计算参数
    float speedWeight;             // 速度惩罚的权重
    float minSpeedRatio;           // 最小速度比率
    float maxSpeedRatio;           // 最大速度比率
    float minSpeedPenalty;         // 最小速度惩罚
    float dtwBandwidthRatio;       // DTW带宽比例
    float similarityThreshold;      // 相似度阈值
    
    [[nodiscard]] static inline Config& getInstance() {
        static Config instance;
        return instance;
    }

private:
    Config() : 
        windowWidth(800),
        windowHeight(600),
        frameRate(30),
        recordInterval(33 * 10000),
        speedWeight(0.3f),
        minSpeedRatio(0.6f),
        maxSpeedRatio(1.4f),
        minSpeedPenalty(0.5f),
        dtwBandwidthRatio(0.3f),
        similarityThreshold(0.6f) {}
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
};

bool ReadConfig(const std::string& filename, std::map<std::string, std::string>& config);

bool InitConfig();

} // namespace kfc

#endif // CONFIG_READER_H