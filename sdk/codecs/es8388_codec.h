#ifndef _ES8388_CODEC_H
#define _ES8388_CODEC_H

#include "audio_codec.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ESP_PLATFORM
// ESP32 平台的 ES8388 硬件编解码器实现

#include "driver/i2c.h"
#include "driver/i2s.h"

// ES8388 寄存器地址
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01
#define ES8388_CHIPPOWER        0x02
#define ES8388_ADCPOWER         0x03
#define ES8388_DACPOWER         0x04
#define ES8388_CHIPLOPOW1       0x05
#define ES8388_CHIPLOPOW2       0x06
#define ES8388_ANAVOLMANAG      0x07
#define ES8388_MASTERMODE       0x08

// ES8388 编解码器实现数据
typedef struct {
    i2c_port_t i2c_port;           // I2C 端口
    i2s_port_t i2s_port;           // I2S 端口
    uint8_t i2c_addr;              // I2C 地址
    int scl_pin;                   // SCL 引脚
    int sda_pin;                   // SDA 引脚
    
    // 音频配置
    int sample_rate;
    int bit_width;
    int channels;
    
    // 状态标志
    bool initialized;
    bool encoder_ready;
    bool decoder_ready;
    
    // 硬件配置
    int input_gain;                // 输入增益
    int output_volume;             // 输出音量
    bool mic_bias_enabled;         // 麦克风偏置使能
    bool adc_enabled;              // ADC 使能
    bool dac_enabled;              // DAC 使能
} ES8388CodecData;

#else
// 非 ESP 平台的 stub 数据结构
typedef struct {
    bool initialized;
    bool encoder_ready;
    bool decoder_ready;
} ES8388CodecData;

#endif // ESP_PLATFORM

// ES8388 编解码器创建函数
audio_codec_t* es8388_codec_create(void);

// ES8388 特定的配置函数
#ifdef ESP_PLATFORM
codec_error_t es8388_codec_set_i2c_config(audio_codec_t* codec, i2c_port_t port, 
                                          int scl_pin, int sda_pin, uint8_t addr);
codec_error_t es8388_codec_set_i2s_config(audio_codec_t* codec, i2s_port_t port);
codec_error_t es8388_codec_set_input_gain(audio_codec_t* codec, int gain);
codec_error_t es8388_codec_set_output_volume(audio_codec_t* codec, int volume);
codec_error_t es8388_codec_enable_mic_bias(audio_codec_t* codec, bool enable);
#endif

#ifdef __cplusplus
}
#endif

#endif // _ES8388_CODEC_H