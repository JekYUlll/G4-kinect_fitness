# 使用说明

## 按键功能

![Kinect](images/me.png)

程序界面顶部有四个功能按键：

### Start/Pause 按钮
- 位置：最左侧
- 功能：开始/暂停动作识别
- 说明：
  - 点击 "Start" 开始进行实时动作识别和相似度计算
  - 再次点击变为 "Pause"，暂停计算
  - 必须先点击 Start 才能使用其他功能

### Record 按钮
- 位置：左数第二个
- 功能：录制标准动作
- 说明：
  - 必须在 Start 状态下才能录制
  - 点击 "Record" 开始录制当前动作
  - 再次点击变为 "Stop"，结束录制并保存
  - 录制的动作会自动保存到配置文件指定的路径

### Print 按钮
- 位置：左数第三个
- 功能：打印当前标准动作数据
- 说明：
  - 用于调试和验证标准动作数据
  - 会在控制台输出详细的关节位置信息

### Play 按钮
- 位置：最右侧
- 功能：播放标准动作
- 说明：
  - 点击 "Play" 开始播放已加载的标准动作
  - 再次点击变为 "Stop"，停止播放
  - 播放时会在屏幕上显示蓝色的标准动作骨骼

## 配置文件

### 文件位置
配置文件为 `data/config.toml`，使用 TOML 格式。首次运行时会自动创建默认配置文件。

### 配置项说明

```toml
# 窗口设置
[window]
width = 800                  # 窗口宽度 (640-1920)
height = 600                 # 窗口高度 (480-1080)

# 帧率设置
[fps]
display = 60                 # 画面显示帧率 (30-120)
record = 30                  # 动作录制帧率 (10-60)
compare = 15                 # 相似度计算帧率 (1-30)

# 动作设置
[action]
standardPath = "data\\sit.dat"  # 标准动作文件路径
bufferSize = 120               # 动作缓冲区大小（帧数）

# 相似度计算参数
[similarity]
difficulty = 3                 # 难度等级 (1-5)，3为标准难度
speedWeight = 0.8              # 速度惩罚权重
minSpeedRatio = 0.7           # 最小速度比率
maxSpeedRatio = 1.3           # 最大速度比率
minSpeedPenalty = 0.05        # 最小速度惩罚
dtwBandwidthRatio = 0.3       # DTW带宽比例 (0.1-1.0)
threshold = 0.6               # 相似度阈值 (0.0-1.0)
similarityHistorySize = 100   # 相似度历史记录容量大小，小于等于0表示无限累加
```

### 参数说明

#### 窗口设置
- `width`/`height`: 程序窗口的尺寸，建议保持默认值

#### 帧率设置
- `display`: 画面刷新率，影响显示流畅度
- `record`: 动作录制时的采样率
- `compare`: 相似度计算频率，影响实时性和CPU占用

#### 动作设置
- `standardPath`: 标准动作文件的保存路径
- `bufferSize`: 动作缓冲区大小，影响计算延迟

#### 相似度计算参数
- `difficulty`: 难度等级，影响评分的严格程度
  - 1: 最容易，较大的容错空间
  - 3: 标准难度，推荐设置
  - 5: 最难，要求极高的精确度
- `speedWeight`: 速度惩罚的权重
- `minSpeedRatio`/`maxSpeedRatio`: 速度比率的允许范围
- `dtwBandwidthRatio`: DTW算法的带宽比例
- `threshold`: 动作匹配的相似度阈值
- `similarityHistorySize`: 历史记录大小，用于计算平均准确率

### 注意事项
1. 修改配置文件后需要重启程序才能生效
2. 不建议将参数调整到极端值，可能影响识别效果
3. 如果程序无法启动，请检查配置文件格式是否正确
