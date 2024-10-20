import base64
import json
import wave
import paho.mqtt.client as mqtt

# MQTT 连接信息
MQTT_BROKER = '192.168.6.178'  # 替换为你的MQTT代理地址
MQTT_PORT = 1883
MQTT_USERNAME = 'sensor'  # 替换为你的用户名
MQTT_PASSWORD = 'sensor'  # 替换为你的密码

# PHOTO_DATA_TOPIC 和 VOICE_DATA_TOPIC
PHOTO_DATA_TOPIC = 'sensor-camera/sensor-camera-20:B2:CE:3F:01:00/photo-data'
VOICE_DATA_TOPIC = 'sensor-camera/sensor-camera-20:B2:CE:3F:01:00/voice-data'

# MQTT 消息处理回调函数
def on_message(client, userdata, msg):
    try:
        # 尝试解析 JSON 数据
        print(len(msg.payload))
        # print(msg.payload)
        json_data = json.loads(msg.payload)
        # print(f"Received message: {json_data}")
        print(msg.topic)
        # 处理图像数据
        if msg.topic == PHOTO_DATA_TOPIC:
            
            base64_image = json_data['data']
            
            image_data = base64.b64decode(base64_image)

            # 保存为图像文件
            with open('output_image.jpg', 'wb') as image_file:
                image_file.write(image_data)

            print("Image data received and saved as output_image.jpg")

        # 处理音频数据
        elif msg.topic == VOICE_DATA_TOPIC:
            # print(json_data)
            base64_audio = json_data['data']
            # print(base64_audio)
            audio_data = base64.b64decode(base64_audio)

            # 保存为音频文件
            with wave.open('output_audio.wav', 'wb') as wav_file:
                nchannels = 1  # 单声道
                sampwidth = 2  # 16位
                framerate = 16000  # 采样率
                nframes = len(audio_data) // sampwidth

                wav_file.setnchannels(nchannels)
                wav_file.setsampwidth(sampwidth)
                wav_file.setframerate(framerate)
                wav_file.setnframes(nframes)

                wav_file.writeframes(audio_data)

            print("Audio data received and saved as output_audio.wav")

    except json.JSONDecodeError:
        print("Failed to decode JSON data")
    except Exception as e:
        print(f"An error occurred: {e}")

# 创建 MQTT 客户端
client = mqtt.Client(protocol=mqtt.MQTTv5)

# 设置用户名和密码
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

# 设置回调函数
client.on_message = on_message

# 连接到 MQTT 代理
client.connect(MQTT_BROKER, MQTT_PORT)

# 订阅图像和音频数据主题
client.subscribe(PHOTO_DATA_TOPIC)
client.subscribe(VOICE_DATA_TOPIC)

# 开始循环
client.loop_forever()
