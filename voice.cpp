#include "voice.h"
#include <base64.h>        // 用于 Base64 编码
#include <ArduinoJson.h>   // 用于处理 JSON 数据

// I2S 麦克风配置
bool init_microphone() {
    // 配置 I2S 驱动
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),  // 主模式并接收数据
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,         // 仅使用右声道
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,             // 中断分配标志
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false
    };

    // I2S 引脚配置
    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,    // I2S BCK 引脚
        .ws_io_num = 25,     // I2S WS 引脚
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 33    // I2S 数据输入引脚
    };

    // 安装 I2S 驱动
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("I2S driver install failed: %d\n", err);
        return false;
    }

    // 设置 I2S 引脚
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("I2S set pin failed: %d\n", err);
        return false;
    }

    return true;  // 初始化成功
}

// 录音并发送语音数据
void record_and_send_voice(PubSubClient& client, const String& voice_data_topic, int duration, int count) {
    const int buffer_size = 1024;  // 定义缓冲区大小
    uint8_t buffer[buffer_size];

    // 每次录音的字节数
    int bytes_to_read = buffer_size;

    // 计算录音的总时长 (毫秒)
    int total_duration_ms = duration * 1000;

    // 录音并发送数据
    for (int i = 0; i < count; i++) {
        int elapsed_time = 0;
        while (elapsed_time < total_duration_ms) {
            size_t bytes_read = 0;
            // 从 I2S 读取音频数据
            i2s_read(I2S_NUM_0, (void*)buffer, bytes_to_read, &bytes_read, portMAX_DELAY);

            // 如果读取成功
            if (bytes_read > 0) {
                // 将音频数据编码为 Base64
                String base64Audio = base64::encode(buffer, bytes_read);

                // 构建 JSON 数据
                StaticJsonDocument<256> doc;
                doc["data"] = base64Audio;  // Base64 编码的音频数据
                doc["index"] = i + 1;       // 当前录音的索引

                // 序列化 JSON 数据
                String jsonData;
                serializeJson(doc, jsonData);

                // 发布 MQTT 消息
                if (client.publish(voice_data_topic.c_str(), jsonData.c_str())) {
                    Serial.println("Voice data sent successfully");
                } else {
                    Serial.println("Failed to send voice data");
                }
            }

            delay(100);  // 控制读取频率
            elapsed_time += 100;
        }

        // 录音之间的间隔
        delay(1000);
    }
}
