# G4-kinect_fitness
### SEU HCI2024 G4 

---

中文
---

 *Kinect Fitness*，命名空间为`kf`。`

默认编译环境为 Visual Studio 2022，使用 C++17 标准。

#### External

- 窗口：  Win32 API
- 日志库：[spdlog](https://github.com/gabime/spdlog)

#### 用法

将`config.txt`放在程序可执行文件同级目录中，配置以下内容：
```cpp
file_path = skeleton_record_20241217_092356.dat
window_width = 800
window_height = 600
frame_rate = 30
```
`file_path`为标准动作数据文件，为相对可执行文件的路径。  
窗口中点击 [record] 按钮后开始录制，默认以`skeleton_record_` + 当前时间戳为文件命名，默认以.dat文件二进制存储。

---

English
---






