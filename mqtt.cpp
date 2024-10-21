#include "mqtt.h"
#include "esp_camera.h"  // 如果需要处理摄像头
#include "camera.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <base64.h>

// 定义 MQTT 服务器地址
const char* mqtt_server = "192.168.6.178";  // 改为在这里定义

// 定义 MQTT 用户名和密码
const char* mqtt_user = "sensor";     // 在这里定义 MQTT 用户名
const char* mqtt_password = "sensor"; // 在这里定义 MQTT 密码

// 设备的唯一标识符
String device_id;  // 在这里定义 device_id

// 获取设备的 MAC 地址
String getDeviceId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[18];  // 17 字符 + 1 个终止符
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);  // 将 MAC 地址转换为 String 类型并返回
}

// 生成客户端 ID
String generateClientId() {
    String prefix = "sensor";  // 您可以自定义前缀
    String deviceType = "camera";  // 设备类型
    String deviceSerial = getDeviceId();  // 设备序列号
    String clientId = prefix + "-" + deviceType + "-" + deviceSerial;  // 生成客户端 ID
    return clientId;
}


// MQTT 主题配置，使用设备 ID 构建不同的主题
const String client_id = generateClientId();
const String take_photo_topic = "sensor-camera/" + client_id + "/take-photo";   // 拍照指令主题
const String photo_data_topic = "sensor-camera/" + client_id + "/photo-data";   // 照片数据主题
const String take_voice_topic = "sensor-camera/" + client_id + "/take-voice";   // 语音录制指令主题
const String voice_data_topic = "sensor-camera/" + client_id + "/voice-data";   // 语音数据主题

// 定义 WiFiClient 和 PubSubClient 实例
WiFiClient espClient;
PubSubClient client(espClient);

// 设置 MQTT 客户端和订阅主题
void setup_mqtt() {
    client.setServer(mqtt_server, 1883);  // 设置 MQTT 服务器
    client.setCallback(mqtt_callback);    // 设置回调函数
    client.setBufferSize(1000000); // 设置缓冲区
}

// 处理 MQTT 连接
void mqtt_reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // 连接到 MQTT 服务器，添加用户名和密码
        if (client.connect(client_id.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("connected");
            // 订阅拍照和录音指令的主题
            client.subscribe(take_photo_topic.c_str());
            client.subscribe(take_voice_topic.c_str());
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);  // 连接失败，5秒后重试
        }
    }
}


// 在 mqtt_callback 中调用回调函数
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // 拍照处理
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

        // 调用拍照并发送照片的函数
        take_and_send_photo(client, photo_data_topic, photo_count);
    }
    // 音频处理
    if (String(topic) == take_voice_topic) {
        Serial.println("Take voice command received");

        // 解析 JSON 数据
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        int duration = doc["duration"];  // 获取录音时长
        int count = doc["count"];        // 获取录音次数
        
        // 调用录音并发送音频数据的函数
        record_and_send_voice(client, voice_data_topic, duration, count);
        Serial.printf("Recording voice for %d seconds, %d times\n", duration, count);
    }
}
