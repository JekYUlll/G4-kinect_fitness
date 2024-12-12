#pragma once
#include <mutex>
#include <Kinect.h>
#include <thread>
#include <fstream>
#include <iostream>
#include <vector>

// ע�͵�����ṹ�壬��Ϊ�Ѿ��� serialize.hpp �ж�����
/*struct JointData {
    CameraSpacePoint position;
    JointType jointType;
};*/

// ���ڴ洢׼ȷ�ʵĽṹ
struct AccuracyData {
    double accuracy;      // �������׼ȷ��
    bool isNewData;       // �Ƿ����¼����׼ȷ��
    std::mutex mtx;       // ������
};

namespace kf {  // ��������ռ�

// ����������Ϊ inline �Ա�����ض���
inline double CalculateAccuracy(const std::vector<JointData>& predictedJoints, 
                              const std::vector<JointData>& targetJoints) 
{
    double totalError = 0.0;
    int jointCount = predictedJoints.size();

    for (int i = 0; i < jointCount; ++i) {
        // ����ÿ���ؽڵ�ŷ����þ������
        double error = sqrt(
            pow(predictedJoints[i].position.X - targetJoints[i].position.X, 2) +
            pow(predictedJoints[i].position.Y - targetJoints[i].position.Y, 2) +
            pow(predictedJoints[i].position.Z - targetJoints[i].position.Z, 2)
        );

        totalError += error;
    }

    // ����ƽ�������Ը�Ϊ��Ȩƽ����
    return totalError / jointCount;
}

}  // namespace kf

class SkeletonRecorder {
public:
    SkeletonRecorder() : m_frameCount(0), m_isRecording(false) {}

    // ��ʼ¼������
    void StartRecording(const std::string& filename) {
        m_isRecording = true;
        m_filename = filename;
        m_frameCount = 0;
    }

    // ֹͣ¼������
    void StopRecording() {
        m_isRecording = false;
        WriteToFile();  // ���д��ʣ�������
    }

    // ÿһ֡����ؽ�����
    void RecordFrame(const Joint* joints, int jointCount) {
        if (m_isRecording) {
            // ����ǰ֡������ӵ�������
            for (int i = 0; i < jointCount; ++i) {
                JointData data = { joints[i].Position, joints[i].JointType };
                m_frameBuffer.push_back(data);
            }

            // ÿ 100 ֡��ÿ���ӣ�����ÿ 1 �룩д��һ������
            ++m_frameCount;
            if (m_frameCount % 100 == 0) {  // ����ÿ 100 ֡д��һ��
                WriteToFile();
                m_frameBuffer.clear();  // ��ջ���
            }
        }
    }

private:
    // �����������д���ļ�
    void WriteToFile() {
        if (!m_isRecording || m_frameBuffer.empty()) {
            return;
        }

        std::ofstream ofs(m_filename, std::ios::app);  // ��׷��ģʽ���ļ�
        if (!ofs.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return;
        }

        for (const auto& data : m_frameBuffer) {
            // ���ؽ�λ�ú͹ؽ����͸�ʽ���� CSV ��ʽ��д��
            ofs << data.jointType << ","
                << data.position.X << ","
                << data.position.Y << ","
                << data.position.Z << "\n";
        }

        ofs.close();
    }

    bool m_isRecording;
    std::string m_filename;
    int m_frameCount;
    std::vector<JointData> m_frameBuffer;  // ���ڻ�������
};
