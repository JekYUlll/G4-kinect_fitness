#ifndef CONFIG_H
#define CONFIG_H

namespace kfc {

// 路径相关定义
#define KF_DATA_DIR "data"          // 录制文件存储目录
#define KFC_CONFIG_FILE "data\\config.toml" // 配置文件路径

// 算法选择
//#define USE_LINEAR_REGRESSION
#define USE_DTW

}

#endif //!CONFIG_H