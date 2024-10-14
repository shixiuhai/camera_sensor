#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "esp_camera.h"
#include "camera.h"  // 包含摄像头配置
#include "wifi_manager.h"   // 导入封装的 WiFi 包
#include "mqtt.h"           // 引入 MQTT 配置
#include "sd_config.h"      // 引入 SD 卡配置

WiFiManager wifiManager; // 实例化 WiFi 管理对象

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // 初始化 SD 卡
    if (!initSDCard()) {
        return;  // 如果 SD 卡初始化失败，返回
    }

    // 初始化摄像头
    if (!init_camera()) {
        return;  // 如果摄像头初始化失败，返回
    }

    // 初始化麦克风
//    if(!init_microphone()){
//      return; // 如果麦克风初始化失败,返回
//    }

    // 启动 WiFi 管理
    wifiManager.begin(); 
    
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
    // Serial.printf("传感器正在运行中\n");
    delay(500);
}
