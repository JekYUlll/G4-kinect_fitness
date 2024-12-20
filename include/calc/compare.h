#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "../KFcommon.h"
#include "../action/ActionTemplate.h"
#include "../action/ActionBuffer.h"

namespace kf {

// 计算两个骨骼点之间的欧氏距离
float calculateJointDistance(const CameraSpacePoint& p1, const CameraSpacePoint& p2);

// 计算高斯核函数值
float gaussianKernel(float distance, float sigma = GAUSSIAN_SIGMA);

// 计算两帧之间的相似度
float calculateFrameSimilarity(const FrameData& frame1, const FrameData& frame2);

// 使用DTW算法计算两个动作序列的相似度
float calculateActionSimilarity(const std::vector<FrameData>& action1, 
                              const std::vector<FrameData>& action2);

// 计算实时动作与模板动作的相似度
float compareWithTemplate(const ActionBuffer& realtimeAction);

// 全局变量
extern std::mutex g_templateMutex;
extern std::unique_ptr<ActionTemplate> g_actionTemplate;

} // namespace kf
