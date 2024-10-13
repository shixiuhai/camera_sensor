#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "esp_camera.h"
#include "camera_config.h"  // 包含摄像头配置
#include "wifi_manager.h"   // 导入封装的 WiFi 包
#include "mqtt.h"           // 引入 MQTT 配置
#include "sd_config.h"      // 引入 SD 卡配置

unsigned long lastCaptureTime = 0; // 上次拍照时间
int imageCount = 1;                // 文件计数
bool camera_sign = false;          // 摄像头状态
bool sd_sign = false;              // SD卡状态

WiFiManager wifiManager; // 实例化 WiFi 管理对象

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // 初始化 SD 卡
    if (!initSDCard()) {
        return;  // 如果 SD 卡初始化失败，返回
    }

    wifiManager.begin(); // 启动 WiFi 管理

    camera_config_t config = get_camera_config();  // 获取摄像头配置
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // 设置和连接 MQTT
    setup_mqtt();

    Serial.println("Setup complete");
}

void loop() {
    if (!client.connected()) {
        mqtt_reconnect();  // 确保 MQTT 连接
    }
    client.loop();  // 处理 MQTT 消息

    // 示例延时
    Serial.printf("Success\n");
    delay(5000);
}