#ifndef CONFIG_H
#define CONFIG_H

namespace kfc {

// 基础配置
//#define WRITE_TO_FILE true
//#define WINDOW_TITLE L"G4-KinectFitnessCoach"
#define ACTION_BUFFER_SIZE 120

// 路径相关定义
#define KF_DATA_DIR "data"          // 录制文件存储目录
#define KF_CONFIG_FILE "config.txt" // 配置文件在根目录

// 算法选择
//#define USE_LINEAR_REGRESSION
#define USE_DTW

}

#endif //!CONFIG_H