#ifndef COMPARE_HPP
#define COMPARE_HPP

#define NOMINMAX
#include <future>
#include <mutex>
#include <cmath> 
#include <limits>
#include <Eigen/Dense>
#include "serialize.h"

namespace kfc {

// 基础类型定义
using Vector3d = Eigen::Vector3f;

// 基础工具函数声明
Vector3d toEigenVector(const CameraSpacePoint& p);
Vector3d getNormalizedBoneVector(const CameraSpacePoint& joint1, const CameraSpacePoint& joint2);
float calculateAngle(const Vector3d& v1, const Vector3d& v2);
float calculateAngleSimilarity(float angle, float sigma);

// 速度相关函数声明
float calculateJointSpeed(const JointData& current, const JointData& prev, float timeInterval);
float calculateSpeedPenalty(float speedRatio);

// 相似度计算核心函数声明
float compareFrames(const FrameData& realFrame, const FrameData& templateFrame);
float postProcessSimilarity(float rawSimilarity, float sensitivity);

// 动作比较相关函数声明
float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate);
std::future<float> compareActionAsync(const ActionBuffer& buffer);

} // namespace kfc

#endif // !COMPARE_HPP
