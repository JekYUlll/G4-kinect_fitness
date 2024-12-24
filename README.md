# G4-kinect_fitness - KFC
### SEU HCI2024 G4 

---

*Kinect Fitness Coach* 是一个基于 Kinect v2 的健身教练系统框架。简称 KFC (认真)。  
主命名空间为`kfc`。

默认编译环境为 Visual Studio 2022，使用 C++17 标准。  
依赖 Windows API 及 Kinect SDK v2，因此仅支持 Windows 10 及以上版本。

#### External

<a href="External/README.md">External</a>.

- 窗口：  Win32 API
- 日志库：[spdlog](https://github.com/gabime/spdlog)
- 数学库：[Eigen 3.4.0](https://eigen.tuxfamily.org/index.php?title=Main_Page)

#### 算法说明

见<a href="docs/README.md">此处</a>文件。

#### 用法

将`config.txt`放在程序可执行文件同级目录中，配置以下内容（后三个参数实际未启用）：
```cpp
file_path = data\\jump.dat
window_width = 800
window_height = 600
frame_rate = 30
```
`file_path`为标准动作数据文件，为相对可执行文件的路径。  
窗口中点击 [record] 按钮后开始录制，默认以`skeleton_record_` + 当前时间戳为文件命名，默认以.dat文件二进制存储。

---

English version <a href="README_en.md">here</a>.






