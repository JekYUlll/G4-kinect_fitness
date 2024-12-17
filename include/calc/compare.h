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

    // 计算两个关节数据的距离
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b);

    // 比较两个动作帧的相似度
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);

    // 比较实时动作缓冲区与标准动作
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // 异步比较动作
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);


}




#endif
