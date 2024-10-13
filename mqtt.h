#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>    // 包含 PubSubClient 库，用于 MQTT 功能
#include <ArduinoJson.h>     // 包含 ArduinoJson 库，用于处理 JSON 数据
#include "base64.h"          // 包含 Base64 库，用于处理 Base64 编码和解码
#include <WiFi.h>

// MQTT 服务器的 IP 地址或主机名
extern const char* mqtt_server; // 改为 extern，确保在其他文件中可以访问

// MQTT 用户名和密码
extern const char* mqtt_user;   // 从其他地方获取 MQTT 用户名
extern const char* mqtt_password; // 从其他地方获取 MQTT 密码

// 设备的唯一标识符
extern String device_id; // 保持为 String 类型，确保在其他文件中可以访问

// MQTT 主题配置，使用设备 ID 构建不同的主题
extern const String take_photo_topic; // 拍照指令主题
extern const String photo_data_topic; // 照片数据主题
extern const String take_voice_topic; // 语音录制指令主题
extern const String voice_data_topic; // 语音数据主题

// 实例化 WiFiClient 和 PubSubClient
extern WiFiClient espClient; // 确保定义了这个实例
extern PubSubClient client;   // 使用 WiFiClient 实例化 PubSubClient

// 函数声明
void setup_mqtt(); 
void mqtt_reconnect(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length); 

#endif // MQTT_H
