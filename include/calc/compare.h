#ifndef COMPARE_HPP
#define COMPARE_HPP

#define NOMINMAX
#include <future>
#include <mutex>
#include <cmath> 
#include <limits>
#include "serialize.h"

#include <Eigen/Dense> // ���ڻع����

namespace kf {

#if defined(USE_LINEAR_REGRESSION)

    // ����һ��λ�����ݵ����Իع����
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues);
    // �Ƚ���������֡�����ƶ�
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);
    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);
    // �첽�Ƚ϶���
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);

#elif defined(USE_DTW)
    // �������޸�Ϊ��ͨ�õ� ��ά�������룬�������Իع���������߼���
    float calculateRegressionError(const std::vector<float>& realValues, const std::vector<float>& templateValues);

    // �Ƚ���������֡
    float compareFrames(const FrameData& realFrame, const FrameData& templateFrame);

    // DTW �ȽϺ���
    // ����������ģ�嶯���е�ÿһ֡���� compareFrames ����֡������㣬��ʹ�ö�̬ʱ�������DTW���㷨�����С����·����
    float computeDTW(const std::vector<FrameData>& realFrames, const std::vector<FrameData>& templateFrames);

    // �Ƚ϶������������׼ģ��
    float compareActionBuffer(const ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // �첽�����Ƚ�
    std::future<float> compareActionAsync(const ActionBuffer& buffer);

#else

    // ���������ؽ����ݵľ���
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b);

    // �Ƚ���������֡�����ƶ�
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);

    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // �첽�Ƚ϶���
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);

#endif

}

#endif // !COMPARE_HPP
