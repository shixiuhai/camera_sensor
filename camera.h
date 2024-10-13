#ifndef CAMERA_H
#define CAMERA_H

#include <esp_camera.h>
#include <PubSubClient.h>

// 声明摄像头配置函数
camera_config_t get_camera_config();

// 声明摄像头初始化函数
bool init_camera();

// 声明拍照并发送照片的函数
void take_and_send_photo(PubSubClient& client, const String& photo_data_topic, int photo_count);

#endif // CAMERA_H
