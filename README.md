# G4-kinect_fitness - KFC
### SEU HCI2024 G4 

---

*Kinect Fitness Coach* 是一个基于 Kinect v2 的健身教练系统框架。简称 KFC (认真)。  
主命名空间为`kfc`。

默认编译环境为 Visual Studio 2022，使用 C++17 标准。  
依赖 Windows API 及 Kinect SDK v2，因此仅支持 Windows 10 及以上版本。

依赖库
---

<a href="External/README.md">External位置</a>。

- 窗口：  Win32 API
- 日志库：[spdlog](https://github.com/gabime/spdlog)
- 数学库：[Eigen 3.4.0](https://eigen.tuxfamily.org/index.php?title=Main_Page)

算法说明
---

见<a href="docs/README.md">此处</a>文件。

用法
---

## 文档

- [使用说明](docs/Usage.md) - 详细的使用指南，包含按键说明和配置文件说明
- [算法说明](docs/Algorithm.md) - 技术实现细节和算法原理

## 快速开始

1. 安装依赖项
2. 运行程序
3. 按照[使用说明](docs/Usage.md)进行操作

## 注意事项

1. 确保Kinect设备正确连接
2. 保持适当的距离（1.5-3米）
3. 避免复杂背景和遮挡

`file_path`为标准动作数据文件，为相对可执行文件的路径。  
窗口中点击 [record] 按钮后开始录制，默认以`skeleton_record_` + 当前时间戳为文件命名，默认以.dat文件二进制存储。

---

English version <a href="README_en.md">here</a>.






