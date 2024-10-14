#ifndef VOICE_H
#define VOICE_H

#include <Arduino.h>
#include <PubSubClient.h>  // 用于 MQTT 通信
#include <ESP_I2S.h>      // 使用 ESP_I2S 库

// 麦克风初始化函数声明
bool init_microphone();

// 录音并发送语音数据的函数声明
void record_and_send_voice(PubSubClient& client, const String& voice_data_topic, int duration, int count);

#endif // VOICE_H
