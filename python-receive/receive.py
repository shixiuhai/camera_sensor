import base64
import json
import wave
import paho.mqtt.client as mqtt

# MQTT 连接信息
MQTT_BROKER = '你的MQTT代理地址'
MQTT_PORT = 1883
VOICE_DATA_TOPIC = '你的音频数据主题'

# MQTT 消息处理回调函数
def on_message(client, userdata, msg):
    # 解析 JSON 数据
    try:
        json_data = json.loads(msg.payload)
        base64_audio = json_data['data']

        # 解码 Base64 数据
        audio_data = base64.b64decode(base64_audio)

        # 保存为 WAV 文件
        with wave.open('output.wav', 'wb') as wav_file:
            # 设置 WAV 文件参数
            nchannels = 1  # 单声道
            sampwidth = 2  # 16位
            framerate = 16000  # 采样率
            nframes = len(audio_data) // sampwidth
            
            wav_file.setnchannels(nchannels)
            wav_file.setsampwidth(sampwidth)
            wav_file.setframerate(framerate)
            wav_file.setnframes(nframes)

            # 写入音频数据
            wav_file.writeframes(audio_data)

        print("Audio data received and saved as output.wav")

    except json.JSONDecodeError:
        print("Failed to decode JSON data")

# 创建 MQTT 客户端
client = mqtt.Client()

# 设置回调函数
client.on_message = on_message

# 连接到 MQTT 代理
client.connect(MQTT_BROKER, MQTT_PORT)

# 订阅音频数据主题
client.subscribe(VOICE_DATA_TOPIC)

# 开始循环
client.loop_forever()
