#pragma once

// Windows & COM
#include <windows.h>
#include <ole2.h>

// Kinect SDK
#if defined(_MSC_VER)
    #include <Kinect.h>
    #pragma comment(lib, "kinect20.lib")
#endif

// DirectX
#if defined(_MSC_VER)
    #include <d2d1.h>
    #include <d2d1helper.h>
    #pragma comment(lib, "d2d1.lib")
#endif

// STL
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include <deque>
#include <filesystem>
#include <fstream>
#include <cstdint>

// Project specific
#include "config.h"

// Forward declarations
namespace kf {
    class KinectManager;
    class SkeletonRenderer;
    class ActionRecorder;
    class SimilarityCalculator;
    class ActionTemplate;
    class ActionBuffer;

    // Type definitions
    enum class JointType {
        SpineBase = 0,
        SpineMid = 1,
        Neck = 2,
        Head = 3,
        ShoulderLeft = 4,
        ElbowLeft = 5,
        WristLeft = 6,
        HandLeft = 7,
        ShoulderRight = 8,
        ElbowRight = 9,
        WristRight = 10,
        HandRight = 11,
        HipLeft = 12,
        KneeLeft = 13,
        AnkleLeft = 14,
        FootLeft = 15,
        HipRight = 16,
        KneeRight = 17,
        AnkleRight = 18,
        FootRight = 19,
        SpineShoulder = 20,
        HandTipLeft = 21,
        ThumbLeft = 22,
        HandTipRight = 23,
        ThumbRight = 24,
        Count = 25
    };

    enum class HandState {
        Unknown = 0,
        NotTracked = 1,
        Open = 2,
        Closed = 3,
        Lasso = 4
    };

    enum class TrackingState {
        NotTracked = 0,
        Inferred = 1,
        Tracked = 2
    };

    struct CameraSpacePoint {
        float X;
        float Y;
        float Z;
    };

    struct JointData {
        JointType type;
        CameraSpacePoint position;
        TrackingState trackingState;
    };

    struct FrameData {
        std::vector<JointData> joints;
        uint64_t timestamp;
    };

    // Global variables
    extern std::mutex g_templateMutex;
    extern std::unique_ptr<ActionTemplate> g_actionTemplate;

    // Utility functions
    bool ensureDirectoryExists(const std::string& path);

    // Serialization functions
    void serialize(std::ofstream& ofs, const JointData& data);
    void deserialize(std::ifstream& ifs, JointData& data);
    void serialize(std::ofstream& ofs, const FrameData& data);
    void deserialize(std::ifstream& ifs, FrameData& data);
} // namespace kf


