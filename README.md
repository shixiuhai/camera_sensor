# camera_sensor ESP32-S3 XIAO
esp32 xiao s3 的mqtt命令下发控制程序
# 使用方案
* 连接esp32-s3 xiao 的ap热点
* 手机浏览器打开 192.168.4.1 进入配置页面
* 输入wifi名称和密码，点击保存
* 代码了mqtt.cpp的对应配置记得自行修改一下
# 描述
* python-receiver.py 接收mqtt命令并执行
* PubSubClient-2.8.0 是修改过的库，直接导入使用
