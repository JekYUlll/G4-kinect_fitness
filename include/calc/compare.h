#ifndef COMPARE_HPP
#define COMPARE_HPP

#define NOMINMAX
#include <future>
#include <mutex>
#include <cmath> 
#include <limits>
#include "serialize.h"

#include <Eigen/Dense> // 用于回归分析

namespace kf {

#if defined(USE_LINEAR_REGRESSION)

    // 计算一组位置数据的线性回归误差
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues);
    // 比较两个动作帧的相似度
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);
    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);
    // 异步比较动作
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);

#elif defined(USE_DTW)
    // 将输入修改为更通用的 单维向量输入，保留线性回归误差计算的逻辑。
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues);

    // 比较两个动作帧
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame);

    // DTW 比较函数
    // 将缓冲区与模板动作中的每一帧调用 compareFrames 进行帧距离计算，并使用动态时间规整（DTW）算法求解最小代价路径。
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames);

    // 比较动作缓冲区与标准模板
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // 异步动作比较
    std::future<float> compareActionAsync(const ActionBuffer& buffer);

#else

    // 计算两个关节数据的距离
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b);

    // 比较两个动作帧的相似度
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // 异步比较动作
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);

#endif

}

#endif // !COMPARE_HPP
