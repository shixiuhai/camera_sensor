#include "voice.h"
#include <base64.h>
#include <ArduinoJson.h>
#include <ESP_I2S.h>  // 使用 ESP_I2S 头文件

I2SClass I2S;  // 创建 I2S 对象

// I2S 麦克风配置
bool init_microphone() {
    // 设置 PDM 时钟和数据引脚
    I2S.setPinsPdmRx(42, 41);

    // 启动 I2S，设置为 PDM 接收模式，采样率 16 kHz，16 位数据，单声道
    if (!I2S.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        Serial.println("Failed to initialize I2S!");
        return false;  // 初始化失败
    }

    return true;  // 初始化成功
}

// 录音并发送语音数据
void record_and_send_voice(PubSubClient& client, const String& voice_data_topic, int duration, int count) {
    const int buffer_size = 1024;  // 定义缓冲区大小
    uint8_t buffer[buffer_size];
    int bytes_filled = 0;          // 记录当前填充的字节数

    // 计算录音的总时长 (毫秒)
    int total_duration_ms = duration * 1000;

    // 录音并发送数据
    for (int i = 0; i < count; i++) {
        int elapsed_time = 0;
        bytes_filled = 0;  // 每次开始新的录音时重置填充字节数

        while (elapsed_time < total_duration_ms) {
            // 从 I2S 读取音频数据
            int sample = I2S.read();  // 读取音频样本
            if (sample != 0) {  // 过滤无效样本
                buffer[bytes_filled++] = sample & 0xFF;  // 将样本的低字节存储到缓冲区
                buffer[bytes_filled++] = (sample >> 8) & 0xFF;  // 将样本的高字节存储到缓冲区
            }

            // 控制读取频率，减少 CPU 占用
            delay(10);  
            elapsed_time += 10;
        }

        // 录音结束后进行 Base64 编码
        if (bytes_filled > 0) {
            String base64Audio = base64::encode(buffer, bytes_filled);

            // 构建 JSON 字符串
            String jsonString = "{\"data\":\"" + base64Audio + "\"}";

            // 发布 MQTT 消息
            if (client.publish(voice_data_topic.c_str(), jsonString.c_str())) {
                Serial.println("Voice data sent successfully");
            } else {
                Serial.println("Failed to send voice data");
            }
        }else{
          
          Serial.println("Voice data is empty");
         }

        // 录音之间的间隔
        delay(1000);
    }
}
