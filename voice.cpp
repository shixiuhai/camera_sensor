#include "voice.h"
#include <base64.h>
#include <ESP_I2S.h>

I2SClass I2S;

// I2S 麦克风配置
bool init_microphone() {
    I2S.setPinsPdmRx(42, 41);
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
    const int buffer_size = 1024;
    int16_t buffer[buffer_size];

    const int total_samples = 16000 * duration;
    const int bytes_per_sample = sizeof(int16_t);

    for (int i = 0; i < count; i++) {
        Serial.printf("Starting recording %d of %d\n", i + 1, count);
        std::vector<int16_t> full_audio_data;
        int samples_read = 0;

        while (samples_read < total_samples) {
            int samples_available = I2S.available() / 2;
            if (samples_available > 0) {
                for (int j = 0; j < samples_available && samples_read < total_samples; j++) {
                    if (samples_read < total_samples) {
                        buffer[j % buffer_size] = I2S.read();
                        samples_read++;
                    }
                }
                full_audio_data.insert(full_audio_data.end(), buffer, buffer + samples_available);
            } else {
                delay(1); // 限制CPU使用
            }

            if (samples_read % 1600 == 0) {
                Serial.printf("Recording progress: %.1f seconds\n", (float)samples_read / 16000);
            }
        }

        Serial.println("Recording finished. Encoding and sending data...");

        // 将所有的 PCM 数据编码为 Base64
        String base64Audio = base64::encode((uint8_t*)full_audio_data.data(), full_audio_data.size() * bytes_per_sample);
        Serial.printf("Base64 encoded length: %d\n", base64Audio.length());

        if (base64Audio.length() > 0) {
            String jsonString = "{\"data\":\"" + base64Audio + "\"}";
            Serial.printf("JSON string length: %d\n", jsonString.length());

            if (get_free_heap() < jsonString.length() + 1000) {
                Serial.println("Warning: Low memory, may not be able to send data");
            }
            Serial.printf("JSON free_heap: %d\n", get_free_heap());

            if (client.publish(voice_data_topic.c_str(), jsonString.c_str())) {
                Serial.println("Voice sent successfully");
            } else {
                Serial.println("Failed to send voice");
                // 可以添加重试逻辑
                client.publish(voice_data_topic.c_str(), jsonString.c_str());
            }
        } else {
            Serial.println("Base64 encoding failed or no data recorded");
        }

        delay(1000);  // 等待 1 秒后开始下一次录音
    }
}
