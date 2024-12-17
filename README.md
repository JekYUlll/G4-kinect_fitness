# G4-kinect_fitness
### SEU HCI2024 G4 

内容为 *Kinect Fitness*，命名空间为`kf`。 

#### 用法

将`config.txt`放在程序可执行文件同级目录中，配置以下内容：
```
file_path = skeleton_record_20241217_092356.dat
window_width = 800
window_height = 600
frame_rate = 30
```
`file_path`为标准动作数据文件路径，默认以.dat文件存储。  
窗口中点击record按钮后开始录制，默认以`skeleton_record_`+当前时间戳为文件命名。



