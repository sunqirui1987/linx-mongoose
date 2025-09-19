# LINX Audio Module

这个模块提供了跨平台的音频录制和播放功能，目前实现了基于PortAudio的Mac版本。

## 功能特性

- 音频录制 (Record)
- 音频播放 (Play)
- 可配置的采样率、声道数、缓冲区大小等参数
- 基于C99标准，兼容性好
- 使用虚函数表实现多态，支持不同硬件平台
- 线程安全的环形缓冲区
- 集成日志系统

## 架构设计

```
AudioInterface (基类)
    ↓
PortAudioMac (Mac平台实现)
```

- `AudioInterface`: 定义了统一的音频接口
- `PortAudioMac`: 基于PortAudio的Mac平台实现

## 依赖安装

### macOS

```bash
# 安装 Homebrew (如果还没有安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装 PortAudio
brew install portaudio
```

## 编译和测试

### 使用 Makefile

```bash
cd sdk/audio/test

# 安装依赖
make install-deps

# 编译
make

# 运行基础测试
make test

# 运行交互式测试 (录音和播放)
make test-interactive
```

### 使用 CMake

```bash
cd sdk/audio
mkdir build && cd build
cmake ..
make
```

## 使用示例

```c
#include "audio/audio_interface.h"
#include "audio/portaudio_mac.h"

int main() {
    // 创建音频接口
    AudioInterface* audio = portaudio_mac_create();
    
    // 初始化
    audio_interface_init(audio);
    
    // 配置音频参数
    // 采样率44.1kHz, 帧大小1024, 双声道, 4个周期, 缓冲区8192, 周期大小2048
    audio_interface_set_config(audio, 44100, 1024, 2, 4, 8192, 2048);
    
    // 开始录音
    audio_interface_record(audio);
    
    // 开始播放
    audio_interface_play(audio);
    
    // 音频数据缓冲区
    short buffer[1024 * 2]; // 1024帧 * 2声道
    
    // 录音和播放循环
    while (running) {
        // 从麦克风读取数据
        if (audio_interface_read(audio, buffer, 1024)) {
            // 写入到扬声器
            audio_interface_write(audio, buffer, 1024);
        }
    }
    
    // 清理资源
    audio_interface_destroy(audio);
    
    return 0;
}
```

## API 参考

### AudioInterface

#### 函数

- `void audio_interface_init(AudioInterface* self)` - 初始化音频接口
- `void audio_interface_set_config(...)` - 设置音频配置
- `bool audio_interface_read(AudioInterface* self, short* buffer, size_t frame_size)` - 读取音频数据
- `bool audio_interface_write(AudioInterface* self, short* buffer, size_t frame_size)` - 写入音频数据
- `void audio_interface_record(AudioInterface* self)` - 开始录音
- `void audio_interface_play(AudioInterface* self)` - 开始播放
- `void audio_interface_destroy(AudioInterface* self)` - 销毁音频接口

#### 配置参数

- `sample_rate`: 采样率 (Hz)，推荐 44100 或 48000
- `frame_size`: 每次处理的帧数，推荐 512, 1024, 2048
- `channels`: 声道数，1=单声道，2=立体声
- `periods`: 周期数，通常 2-8
- `buffer_size`: 缓冲区大小（样本数）
- `period_size`: 周期大小（样本数）

### PortAudioMac

#### 创建实例

```c
AudioInterface* portaudio_mac_create(void);
```

## 注意事项

1. **权限**: 在macOS上，应用可能需要麦克风权限
2. **延迟**: 较小的frame_size会降低延迟但增加CPU使用率
3. **缓冲区**: 适当的缓冲区大小可以避免音频断续
4. **线程安全**: 所有API都是线程安全的
5. **错误处理**: 检查返回值和日志输出

## 扩展支持

要添加新的平台支持（如Linux ALSA、Windows WASAPI），只需：

1. 创建新的实现文件（如 `alsa_linux.c`）
2. 实现 `AudioInterfaceVTable` 中的所有函数
3. 提供创建函数（如 `alsa_linux_create()`）

## 故障排除

### 编译错误

- 确保安装了PortAudio: `brew install portaudio`
- 检查头文件路径是否正确

### 运行时错误

- 检查音频设备是否可用
- 确认应用有麦克风权限
- 查看日志输出获取详细错误信息

### 音频质量问题

- 调整采样率和缓冲区大小
- 检查音频设备设置
- 避免音频反馈（使用耳机）