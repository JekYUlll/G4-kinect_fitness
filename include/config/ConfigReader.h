#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#define NOMINMAX

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdint>

#include "calc/serialize.h"
#include "KFcommon.h"

namespace kfc {

// 编译时字符串哈希
constexpr uint32_t hash_str(const char* str, size_t n) {
    return n == 0 ? 5381 : (hash_str(str, n-1) * 33) ^ str[n-1];
}

constexpr uint32_t operator"" _hash(const char* str, size_t n) {
    return hash_str(str, n);
}

struct Config {
    // 基础配置
    std::string dataDir;            // 数据目录
    std::string configFile;         // 配置文件路径
    std::string standardPath; // 标准动作文件路径
    int windowWidth;               // 窗口宽度
    int windowHeight;              // 窗口高度
    
    // 帧率配置
    int displayFPS;                // 画面显示帧率
    int recordFPS;                 // 动作录制帧率
    int compareFPS;                // 相似度计算帧率
    
    // 动作缓冲区配置
    int actionBufferSize;          // 动作缓冲区大小（帧数）
    
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

    // 获取各种间隔时间（100纳秒）
    [[nodiscard]] inline INT64 getDisplayInterval() const { return static_cast<INT64>(10000000.0 / displayFPS); }
    [[nodiscard]] inline INT64 getRecordInterval() const { return static_cast<INT64>(10000000.0 / recordFPS); }
    [[nodiscard]] inline INT64 getCompareInterval() const { return static_cast<INT64>(10000000.0 / compareFPS); }

    static bool Init(const std::string& configPath);

private:
    static bool Read(const std::string& filename, std::map<std::string, std::string>& config);

    Config() : 
        windowWidth(800),
        windowHeight(600),
        displayFPS(60),
        recordFPS(30),
        compareFPS(10),
        actionBufferSize(120),     // 默认120帧
        speedWeight(0.3f),
        minSpeedRatio(0.6f),
        maxSpeedRatio(1.4f),
        minSpeedPenalty(0.5f),
        dtwBandwidthRatio(0.3f),
        similarityThreshold(0.6f) {}
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
};

} // namespace kfc
#endif // CONFIG_READER_H
