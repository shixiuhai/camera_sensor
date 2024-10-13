#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera.h"
#include "camera_pins.h"
#include <esp_camera.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <base64.h>


// 获取摄像头配置
camera_config_t get_camera_config() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_UXGA;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        config.frame_size = FRAMESIZE_240X240;
    }

    return config;
}

// 初始化摄像头
bool init_camera() {
    camera_config_t config = get_camera_config();  // 获取摄像头配置
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;  // 返回失败
    }
    return true;  // 返回成功
}

// 拍照并发送照片的函数
void take_and_send_photo(PubSubClient& client, const String& photo_data_topic, int photo_count) {
    for (int i = 0; i < photo_count; i++) {
        camera_fb_t *fb = esp_camera_fb_get();  // 获取摄像头的图像帧
        if (!fb) {
            Serial.println("Failed to get camera frame buffer");
            return;
        }

        Serial.printf("Frame buffer acquired: %d bytes\n", fb->len);

        // 将图像数据编码为 Base64
        String base64Image = base64::encode(fb->buf, fb->len);  // 获取 Base64 编码的结果
        Serial.printf("Base64 encoded length: %d\n", base64Image.length());

        if (base64Image.length() > 0) {
            // 构建 JSON 字符串
            String jsonString = "{\"data\":\"" + base64Image + "\"}";  // 使用 String 构建 JSON

            // 发布到 MQTT 主题
            if (client.publish(photo_data_topic.c_str(), jsonString.c_str())) {
                Serial.println("Photo sent successfully");
            } else {
                Serial.println("Failed to send photo");
            }
        } else {
            Serial.println("Base64 encoding failed");
        }

        esp_camera_fb_return(fb);  // 释放帧缓冲
        delay(500);  // 延时 500 毫秒
    }
}
