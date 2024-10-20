#include "voice.h"
#include <base64.h>
#include <ESP_I2S.h>

I2SClass I2S;

// I2S 麦克风配置
bool init_microphone() {
    // 设置 PDM 时钟和数据引脚
    I2S.setPinsPdmRx(42, 41);
    // 启动 I2S，设置为 PDM 接收模式，采样率 16 kHz，16 位数据，单声道
    if (!I2S.begin(I2S_MODE_PDM_RX, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        Serial.println("Failed to initialize I2S!");
        return false;
    }
    return true;
}

// 获取当前可用堆内存
uint32_t get_free_heap() {
    return esp_get_free_heap_size();
}

// 录音并发送语音数据
void record_and_send_voice(PubSubClient& client, const String& voice_data_topic, int duration, int count) {
    const int buffer_size = 1024; // 缓冲区大小（采样点）
    int16_t buffer[buffer_size];
    
    const int total_samples = 16000 * duration;  // 总采样点数
    const int bytes_per_sample = sizeof(int16_t);  // 每个采样点的字节数（2 字节）
    
    for (int i = 0; i < count; i++) {
        Serial.printf("Starting recording %d of %d\n", i + 1, count);
        
        // 创建一个动态数组，用于保存所有的 PCM 数据
        std::vector<int16_t> full_audio_data;
        int samples_read = 0;
        int buffer_fill = 0;  // 记录缓冲区中填充的数据量
        while (samples_read < total_samples) {
            int samples_available = I2S.available() / 2;  // I2S.available() 返回字节数，除以 2 得到采样点数
            if (samples_available > 0) {
                for (int j = 0; j < samples_available && buffer_fill < buffer_size; j++) {
                    buffer[buffer_fill++] = I2S.read();
                    samples_read++;
                    if (buffer_fill >= buffer_size) {
                        // 将缓冲区中的数据添加到 full_audio_data 中
                        full_audio_data.insert(full_audio_data.end(), buffer, buffer + buffer_fill);
                        buffer_fill = 0;  // 清空缓冲区
                    }
                }
            }
            // 打印录音进度
            if (samples_read % 1600 == 0) {
                Serial.printf("Recording progress: %.1f seconds\n", (float)samples_read / 16000);
            }
        }
        // 处理缓冲区中剩余的数据
        if (buffer_fill > 0) {
            full_audio_data.insert(full_audio_data.end(), buffer, buffer + buffer_fill);
        }

        Serial.println("Recording finished. Encoding and sending data...");

        // 将所有的 PCM 数据编码为 Base64
        String base64Audio = base64::encode((uint8_t*)full_audio_data.data(), full_audio_data.size() * bytes_per_sample);
        Serial.printf("Base64 encoded length: %d\n", base64Audio.length());
        
        // 将 Base64 编码的数据封装成 JSON 字符串并通过 MQTT 发送
        if (base64Audio.length() > 0) {
            String jsonString = "{\"data\":\"" + base64Audio + "\"}";
            Serial.printf("JSON string length: %d\n", jsonString.length());

            // 检查是否有足够的内存来发送数据
            if (get_free_heap() < jsonString.length() + 1000) {  // 预留 1000 字节的安全余量
                Serial.println("Warning: Low memory, may not be able to send data");
            }

            Serial.printf("JSON free_heap: %d\n", get_free_heap());
            
            // 发布到 MQTT 主题
            if (client.publish(voice_data_topic.c_str(), jsonString.c_str())) {
                Serial.println("Voice sent successfully");
            } else {
                Serial.println("Failed to send voice");
            }
        } else {
            Serial.println("Base64 encoding failed or no data recorded");
        }
        delay(1000);  // 等待 1 秒后开始下一次录音
    }
}
