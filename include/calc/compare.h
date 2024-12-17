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

    // ���������ؽ����ݵľ���
    float calculateJointDistance(const kf::JointData& a, const kf::JointData& b);

    // �Ƚ���������֡�����ƶ�
    float compareFrames(const kf::FrameData& realFrame, const kf::FrameData& templateFrame);

    // �Ƚ�ʵʱ�������������׼����
    float compareActionBuffer(const kf::ActionBuffer& buffer, const ActionTemplate& actionTemplate);

    // �첽�Ƚ϶���
    std::future<float> compareActionAsync(const kf::ActionBuffer& buffer);


}




#endif
