#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "../KFcommon.h"
#include "../action/ActionTemplate.h"
#include "../action/ActionBuffer.h"

namespace kf {

// ��������������֮���ŷ�Ͼ���
float calculateJointDistance(const CameraSpacePoint& p1, const CameraSpacePoint& p2);

// �����˹�˺���ֵ
float gaussianKernel(float distance, float sigma = GAUSSIAN_SIGMA);

// ������֮֡������ƶ�
float calculateFrameSimilarity(const FrameData& frame1, const FrameData& frame2);

// ʹ��DTW�㷨���������������е����ƶ�
float calculateActionSimilarity(const std::vector<FrameData>& action1, 
                              const std::vector<FrameData>& action2);

// ����ʵʱ������ģ�嶯�������ƶ�
float compareWithTemplate(const ActionBuffer& realtimeAction);

// ȫ�ֱ���
extern std::mutex g_templateMutex;
extern std::unique_ptr<ActionTemplate> g_actionTemplate;

} // namespace kf
