#ifndef CONFIG_H
#define CONFIG_H

namespace kfc {

// 基础配置
//#define WRITE_TO_FILE true
//#define WINDOW_TITLE L"G4-KinectFitnessCoach"
#define ACTION_BUFFER_SIZE 120

// 路径相关定义
#define KF_DATA_DIR "data"          // 录制文件存储目录
#define KF_CONFIG_FILE "data\\config.toml" // 配置文件路径

// 算法选择
//#define USE_LINEAR_REGRESSION
#define USE_DTW

// 相似度计算参数
#define SPEED_WEIGHT 0.3f           // 速度惩罚的权重系数 (0.0-1.0)

}

#endif //!CONFIG_H