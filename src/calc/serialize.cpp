#include "calc/serialize.h"

namespace kfc {

    // ȫ�ֱ���
    std::mutex templateMutex;
    std::unique_ptr<ActionTemplate> g_actionTemplate;

    // ���л����ļ�
    void JointData::serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&type), sizeof(type));
        out.write(reinterpret_cast<const char*>(&position), sizeof(position));
        out.write(reinterpret_cast<const char*>(&trackingState), sizeof(trackingState));
    }

    // ���ļ������л�
    void JointData::deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&type), sizeof(type));
        in.read(reinterpret_cast<char*>(&position), sizeof(position));
        in.read(reinterpret_cast<char*>(&trackingState), sizeof(trackingState));
    }

    // ���л����ļ�
    void FrameData::serialize(std::ofstream& out) const {
        out.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
        size_t jointCount = joints.size();
        out.write(reinterpret_cast<const char*>(&jointCount), sizeof(jointCount));
        for (const auto& joint : joints) {
            joint.serialize(out);
        }
    }

    // ���ļ������л�
    void FrameData::deserialize(std::ifstream& in) {
        in.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        size_t jointCount;
        in.read(reinterpret_cast<char*>(&jointCount), sizeof(jointCount));
        joints.resize(jointCount);
        for (auto& joint : joints) {
            joint.deserialize(in);
        }
    }

    // ���л�һ֡�Ĺ������ݵ��ļ�
    bool SaveFrame(const std::string& filename, const FrameData& frame, bool append) {
        try {
            std::ofstream out(filename, append ? (std::ios::binary | std::ios::app) : std::ios::binary);
            if (!out) {
                return false;
            }
            frame.serialize(out);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // ���ļ���ȡһ֡��������
    bool LoadFrame(const std::string& filename, FrameData& frame) {
        try {
            std::ifstream in(filename, std::ios::binary);
            if (!in) {
                return false;
            }
            frame.deserialize(in);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // ���캯�������ر�׼�����ļ�
    ActionTemplate::ActionTemplate(const std::string& filePath) {
        _frames = std::make_unique<std::vector<kfc::FrameData>>();
        
        // ȷ��Ŀ¼����
        if (!kfc::ensureDirectoryExists()) {
            LOG_E("�޷�ȷ��Ŀ¼�ṹ����");
            throw std::runtime_error("Failed to ensure directory structure");
        }

        // ��ȡ����·��
        std::string fullPath = kfc::getStandardActionPath(filePath);
        //LOG_D("��׼�����ļ�����·��: {}", fullPath);
        
        if (!loadFromFile(fullPath)) {
            LOG_E("�޷����ļ����ر�׼����: {}", fullPath);
            throw std::runtime_error("Failed to load action template from file: " + fullPath);
        }
    }

    // ���ر�׼��������
    bool ActionTemplate::loadFromFile(const std::string& filename) {
        try {
            //std::string filepath = std::string(KF_DATA_DIR) + "\\" + filename;
            std::ifstream in(filename, std::ios::binary);
            if (!in) {
                LOG_E("�޷��򿪱�׼�����ļ�: {}", filename);
                return false;
            }

            _frames->clear();
            size_t frameCount = 0;
            while (in.peek() != EOF) {
                kfc::FrameData frame;
                frame.deserialize(in);
                _frames->push_back(frame);
                frameCount++;
            }

            in.close();
            LOG_I("�ɹ����ر�׼�����ļ�: {}", filename);
            LOG_D("�ܹ�����֡��: {}", frameCount);
            LOG_D("��һ֡�ؽ���: {}", _frames->empty() ? 0 : _frames->front().joints.size());

            return true;
        }
        catch (const std::exception& e) {
            LOG_E("���ر�׼�����ļ�ʱ�����쳣: {}", e.what());
            return false;
        }
        catch (...) {
            LOG_E("���ر�׼�����ļ�ʱ����δ֪�쳣");
            return false;
        }
    }

    // �ѱ�׼������ӡ����־
    void ActionTemplate::PrintData() const {
        for (size_t i = 0; i < _frames->size(); ++i) {
            const auto& frame = (*_frames)[i];
            LOG_D("Frame {} - Timestamp: {}", i, frame.timestamp);
            LOG_D("Number of joints: {}", frame.joints.size());

            for (const auto& joint : frame.joints) {
                LOG_T("  Joint Type: {}, Position: ({:.2f}, {:.2f}, {:.2f}), TrackingState: {}",
                    joint.type, joint.position.X, joint.position.Y, joint.position.Z, joint.trackingState);
            }
        }
    }

    // �첽���ر�׼����
    std::future<bool> loadStandardActionAsync(const std::string& filePath) {
        return std::async(std::launch::async, [&]() {
            std::lock_guard<std::mutex> lock(templateMutex);
            return g_actionTemplate->loadFromFile(filePath);
            });
    }


}