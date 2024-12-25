# G4-Kinect: Real-time Action Scoring System

A real-time action scoring system based on Kinect v2, designed to capture and evaluate the similarity between user actions and standard actions.

Student project, for entertainment purposes only.

## System Requirements

### Hardware Requirements
- Kinect v2 sensor
- USB 3.0 port
- Processor: Dual-core 3.1GHz or higher
- Memory: 4GB or higher
- Graphics: DirectX 11 compatible

### Software Requirements
1. Windows 10 64-bit or higher
2. [Kinect for Windows SDK v2.0](https://www.microsoft.com/en-us/download/details.aspx?id=44561)
3. [Visual C++ Redistributable 2019](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Quick Start

1. Install Required Components:
   - Install Kinect for Windows SDK v2.0
   - Install Visual C++ Redistributable 2019
   
2. Hardware Setup:
   - Connect Kinect v2 sensor to USB 3.0 port
   - Wait for Windows to install device drivers
   
3. Run the Program:
   - Extract `G4-Kinect-v1.0.0-win64.zip`
   - Run `kinect_fitness.exe`

## Directory Structure

```
G4-Kinect/
├── include/            # Header files
│   ├── core/          # Core functionality
│   │   ├── application.h
│   │   ├── common.h
│   │   └── utils.h
│   ├── ui/            # UI related
│   │   └── window.h
│   ├── calc/          # Calculation and algorithms
│   │   ├── compare.h
│   │   └── serialize.h
│   ├── log/           # Logging functionality
│   │   └── logger.h
│   └── config/        # Configuration
│       └── config.h
├── src/               # Source files
│   ├── core/          # Core implementation
│   │   ├── application.cpp
│   │   └── common.cpp
│   ├── ui/            # UI implementation
│   │   └── window.cpp
│   ├── calc/          # Algorithm implementation
│   │   ├── compare.cpp
│   │   └── serialize.cpp
│   ├── log/           # Logger implementation
│   │   └── logger.cpp
│   └── main.cpp       # Program entry
├── data/              # Data files
│   ├── config.toml    # Configuration file
│   └── standard/      # Standard action files
├── docs/              # Documentation
│   ├── Algorithm.md   # Algorithm details
│   └── Usage.md       # Usage guide
├── External/          # External dependencies
│   ├── Eigen/         # Eigen library
│   └── spdlog/        # Logging library
└── README.md          # This file
```

*release*

```
G4-Kinect/
├── kinect_fitness.exe    # Main program
├── data/               # Data directory
│   ├── config.toml    # Configuration file
│   ├── standard/      # Standard action files directory
│   └── record/       # Recorded action files directory
└── README.md          # This file
```

## Basic Operations

### Interface Buttons

1. **Start/Pause Button**
   - Location: Leftmost
   - Function: Start/Pause action recognition
   - Notes:
     - Click "Start" to begin real-time action recognition
     - Click again to pause
     - Must be started before using other functions

2. **Record Button**
   - Location: Second from left
   - Function: Record standard action
   - Notes:
     - Only works when started
     - Click to start recording
     - Click again to stop and save

3. **Print Button**
   - Location: Third from left
   - Function: Print current action data
   - Notes:
     - For debugging and verification
     - Outputs detailed joint position information

4. **Play Button**
   - Location: Rightmost
   - Function: Play standard action
   - Notes:
     - Click to play loaded standard action
     - Click again to stop
     - Shows blue skeleton for standard action

## Configuration File

### Location
Configuration file is located at `data/config.toml`, using TOML format. Created automatically on first run.

### Parameters

```toml
# Window Settings
[window]
width = 800                  # Window width (640-1920)
height = 600                 # Window height (480-1080)

# Frame Rate Settings
[fps]
display = 60                 # Display frame rate (30-120)
record = 30                  # Recording frame rate (10-60)
compare = 15                 # Comparison frame rate (1-30)

# Action Settings
[action]
standardPath = "data\\sit.dat"  # Standard action file path
bufferSize = 120               # Action buffer size (frames)

# Similarity Parameters
[similarity]
difficulty = 3                 # Difficulty level (1-5)
speedWeight = 0.8              # Speed penalty weight
minSpeedRatio = 0.7           # Minimum speed ratio
maxSpeedRatio = 1.3           # Maximum speed ratio
minSpeedPenalty = 0.05        # Minimum speed penalty
dtwBandwidthRatio = 0.3       # DTW bandwidth ratio (0.1-1.0)
threshold = 0.6               # Similarity threshold (0.0-1.0)
similarityHistorySize = 100   # History size for similarity
```

## Algorithm Details

### Position Independence
- Uses relative positions with spine as reference
- Normalizes for body size differences
- Independent of camera position

### Angle Similarity Calculation

1. **Euclidean Distance**: For joint point distances
```cpp
float calculateJointDistance(const CameraSpacePoint& a, const CameraSpacePoint& b) {
    float dx = a.X - b.X;
    float dy = a.Y - b.Y;
    float dz = a.Z - b.Z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
```

2. **Cosine Similarity**: For bone vector angles
```cpp
float calculateAngle(const Vector3d& v1, const Vector3d& v2) {
    float cosAngle = v1.normalized().dot(v2.normalized());
    cosAngle = std::min(1.0f, std::max(-1.0f, cosAngle));
    return std::acos(cosAngle);
}
```

3. **Gaussian Kernel**: Maps angle differences to similarity
```cpp
float calculateAngleSimilarity(float angle, float sigma = 0.8f) {
    return std::exp(-angle * angle / (2.0f * sigma * sigma));
}
```

### DTW (Dynamic Time Warping) Algorithm

1. **Sakoe-Chiba Band Constraint**
- Limits DTW path to a band around the diagonal
- Reduces complexity from O(MN) to O(M·r)
- Maintains temporal consistency

2. **Dynamic Programming Matrix**
- Uses Eigen matrices for efficient computation
- Considers three operations: insert, delete, match
- Applies bandwidth constraints for optimization

3. **Normalization**
- Distance normalization for sequence length differences
- Sigmoid mapping for better discrimination
- Difficulty-based adjustments

## Notes

1. Environment:
   - Requires administrator privileges
   - Ensure firewall/antivirus allows program
   
2. Usage Tips:
   - Use in well-lit environment
   - Avoid direct sunlight on sensor
   - Keep sensor level
   - Ensure clear view of full body

## Technical Support

For issues, please submit to GitHub Issues:
[https://github.com/yourusername/G4-Kinect/issues](https://github.com/yourusername/G4-Kinect/issues)

## License

This software is licensed under the MIT License. See LICENSE file for details.
