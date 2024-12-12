#pragma once
#include <mutex>
#include <Kinect.h>
#include <thread>
#include <fstream>
#include <iostream>
#include <vector>

struct JointData {
    CameraSpacePoint position;
    JointType jointType;
};

// 用于存储准确率的结构
struct AccuracyData {
    double accuracy;      // 计算出的准确率
    bool isNewData;       // 是否有新计算的准确率
    std::mutex mtx;       // 互斥锁
};

//a.基于每个关节的误差（欧几里得距离）
//我们可以通过计算每个关节的位置误差（通常是欧几里得距离），然后根据所有关节的误差来计算整体的准确率。简单地说，首先计算每个关节的位置误差，然后对所有关节的误差取平均值或加权平均。
//
//计算步骤：
//
//对每一帧的每个关节，计算预测关节位置与标准关节位置之间的误差。
//将每个关节的误差汇总（可以是加权平均，考虑到某些关节可能更加重要）。
//通过对整个动作序列的多个帧误差进行累计，计算出整体的准确率。
double CalculateAccuracy(const std::vector<JointData>& predictedJoints, const std::vector<JointData>& targetJoints) {
    double totalError = 0.0;
    int jointCount = predictedJoints.size();

    for (int i = 0; i < jointCount; ++i) {
        // 计算每个关节的欧几里得距离误差
        double error = sqrt(
            pow(predictedJoints[i].position.X - targetJoints[i].position.X, 2) +
            pow(predictedJoints[i].position.Y - targetJoints[i].position.Y, 2) +
            pow(predictedJoints[i].position.Z - targetJoints[i].position.Z, 2)
        );

        totalError += error;
    }

    // 计算平均误差（可以改为加权平均）
    return totalError / jointCount;
}

//void CalculateAndUpdateAccuracy(const Joint* joints, int jointCount) {
//    // 计算准确率
//    double accuracy = CalculateAccuracy(joints, jointCount);
//
//    // 将准确率存入共享数据结构
//    AccuracyData accuracyData;
//    std::lock_guard<std::mutex> lock(accuracyData.mtx);
//    accuracyData.accuracy = accuracy;
//    accuracyData.isNewData = true;  // 表示新数据已经计算完成
//}

class SkeletonRecorder {
public:
    SkeletonRecorder() : m_frameCount(0), m_isRecording(false) {}

    // 开始录制数据
    void StartRecording(const std::string& filename) {
        m_isRecording = true;
        m_filename = filename;
        m_frameCount = 0;
    }

    // 停止录制数据
    void StopRecording() {
        m_isRecording = false;
        WriteToFile();  // 最后写入剩余的数据
    }

    // 每一帧处理关节数据
    void RecordFrame(const Joint* joints, int jointCount) {
        if (m_isRecording) {
            // 将当前帧数据添加到缓存中
            for (int i = 0; i < jointCount; ++i) {
                JointData data = { joints[i].Position, joints[i].JointType };
                m_frameBuffer.push_back(data);
            }

            // 每 100 帧或每秒钟（例如每 1 秒）写入一次数据
            ++m_frameCount;
            if (m_frameCount % 100 == 0) {  // 比如每 100 帧写入一次
                WriteToFile();
                m_frameBuffer.clear();  // 清空缓存
            }
        }
    }

private:
    // 将缓存的数据写入文件
    void WriteToFile() {
        if (!m_isRecording || m_frameBuffer.empty()) {
            return;
        }

        std::ofstream ofs(m_filename, std::ios::app);  // 以追加模式打开文件
        if (!ofs.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return;
        }

        for (const auto& data : m_frameBuffer) {
            // 将关节位置和关节类型格式化成 CSV 格式并写入
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
    std::vector<JointData> m_frameBuffer;  // 用于缓存数据
};
