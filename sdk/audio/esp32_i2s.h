#ifndef ESP32_I2S_H
#define ESP32_I2S_H

#include "audio_interface.h"

#ifdef ESP_PLATFORM
#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/**
 * ESP32 I2S implementation data structure
 */
typedef struct {
    i2s_port_t i2s_port;
    i2s_config_t i2s_config;
    i2s_pin_config_t pin_config;
    
    // Ring buffers for audio data
    short* record_buffer;
    short* play_buffer;
    size_t record_buffer_size;
    size_t play_buffer_size;
    size_t record_read_pos;
    size_t record_write_pos;
    size_t play_read_pos;
    size_t play_write_pos;
    
    // FreeRTOS synchronization
    QueueHandle_t record_queue;
    QueueHandle_t play_queue;
    TaskHandle_t record_task;
    TaskHandle_t play_task;
    
    // State flags
    bool record_task_running;
    bool play_task_running;
} ESP32I2SData;

#else
// Stub data structure for non-ESP32 platforms
typedef struct {
    bool initialized;
    bool recording;
    bool playing;
} ESP32I2SData;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create ESP32 I2S implementation
 * @return AudioInterface instance or NULL on failure
 */
AudioInterface* esp32_i2s_create(void);

/**
 * ESP32 I2S record task
 */
void esp32_i2s_record_task(void* pvParameters);

/**
 * ESP32 I2S play task
 */
void esp32_i2s_play_task(void* pvParameters);

#ifdef __cplusplus
}
#endif

#endif // ESP32_I2S_H