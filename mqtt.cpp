#include "mqtt.h"
#include "esp_camera.h"  // 如果需要处理摄像头
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <base64.h>

// ...（其他代码保持不变）...

// MQTT 回调函数，处理接收到的消息
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    if (String(topic) == take_photo_topic) {
        Serial.println("Take photo command received");

        // 解析 JSON 数据
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        int photo_count = doc["count"];  // 获取需要拍照的数量
        Serial.printf("Taking %d photos\n", photo_count);

        // 拍照并发送照片数据
        for (int i = 0; i < photo_count; i++) {
            camera_fb_t *fb = esp_camera_fb_get();  // 获取摄像头的图像帧
            if (!fb) {
                Serial.println("Failed to get camera frame buffer");
                return;
            }

            // 将图像数据编码为 Base64
            String base64Image = base64::encode(fb->buf, fb->len);

            // 创建 JSON 对象并将 Base64 字符串放入其中
            StaticJsonDocument<300> jsonDoc;
            jsonDoc["data"] = base64Image;

            // 将 JSON 对象序列化为字符串
            String jsonString;
            serializeJson(jsonDoc, jsonString);

            // 通过 MQTT 发布 JSON 字符串
            if (client.publish(photo_data_topic.c_str(), jsonString.c_str())) {
                Serial.println("Image sent successfully");
            } else {
                Serial.println("Failed to send image");
            }

            esp_camera_fb_return(fb);  // 释放帧缓冲
            delay(1000);  // 延时 1 秒
        }
    }

    // 语音录制指令的处理逻辑
    // if (String(topic) == take_voice_topic) {
    //     Serial.println("Take voice command received");
    //     // 解析并处理语音录制指令
    // }
}
