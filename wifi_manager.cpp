#include "wifi_manager.h"
#include <SD.h>

#define CONFIG_FILE "/wifi_config.txt"  // 配置文件的路径
#define LED_PIN 2                        // LED引脚号
#define MAX_CONNECT_ATTEMPTS 3           // 最大WiFi连接尝试次数
#define CONNECT_TIMEOUT 10000            // WiFi连接超时时间（毫秒）

// WiFiManager构造函数
WiFiManager::WiFiManager() : server(80) {
    // 在构造函数中可以不需要 EEPROM 的初始化
}

// 开始WiFi管理过程
void WiFiManager::begin() {
    pinMode(LED_PIN, OUTPUT);
    Serial.println("WiFiManager 开始初始化...");
    Serial.println("检查配置状态: " + String(isConfigured() ? "已配置" : "未配置"));
    while (!connectWiFi()) {
        Serial.println("无法连接到WiFi。进入配置模式...");
        startConfigMode();
    }
}

// 保存WiFi配置到SD卡
void WiFiManager::saveWiFiConfig(const char* ssid, const char* password) {
    Serial.println("正在保存WiFi配置...");

    File configFile = SD.open(CONFIG_FILE, FILE_WRITE);
    if (!configFile) {
        Serial.println("无法打开配置文件进行写入");
        return;
    }

    // 写入SSID和密码
    configFile.println(ssid);
    configFile.println(password);
    configFile.close();
    Serial.println("配置已保存到SD卡");

    // 验证写入
    char saved_ssid[32] = {0};
    char saved_password[64] = {0};
    loadWiFiConfig(saved_ssid, saved_password);
    Serial.println("验证保存的配置:");
    Serial.println("SSID: " + String(saved_ssid));
    Serial.println("Password: " + String(saved_password));
}

// 从SD卡加载WiFi配置
void WiFiManager::loadWiFiConfig(char* ssid, char* password) {
    Serial.println("正在加载WiFi配置...");

    File configFile = SD.open(CONFIG_FILE);
    if (!configFile) {
        Serial.println("无法打开配置文件进行读取");
        return;
    }

    // 读取SSID和密码
    String ssidStr = configFile.readStringUntil('\n');
    String passwordStr = configFile.readStringUntil('\n');
    configFile.close();
    // 移除字符串中的空白字符
    ssidStr.trim();
    passwordStr.trim();
    // 转换为数组
    ssidStr.toCharArray(ssid, 32);
    passwordStr.toCharArray(password, 64);

    Serial.println("加载的SSID: " + String(ssid));
    Serial.println("加载的密码: " + String(password));
}

// 检查配置是否存在
bool WiFiManager::isConfigured() {
    char ssid[32] = {0};
    char password[64] = {0};
    loadWiFiConfig(ssid, password);
    return strlen(ssid) > 0 && strlen(password) > 0;
}

// 尝试连接WiFi
bool WiFiManager::connectWiFi() {
    if (!isConfigured()) {
        Serial.println("WiFi尚未配置");
        return false;
    }

    char ssid[32] = {0};
    char password[64] = {0};
    loadWiFiConfig(ssid, password);

    if (strlen(ssid) == 0 || strlen(password) == 0) {
        Serial.println("无效的SSID或密码");
        return false;
    }
    Serial.println("连接的SSID: " + String(ssid) + " 长度: " + String(strlen(ssid)));
    Serial.println("连接的密码: " + String(password) + " 长度: " + String(strlen(password)));

    Serial.println("尝试连接到WiFi: " + String(ssid));
    WiFi.mode(WIFI_STA); // 设置wifi连接热点的模式
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (attempts < MAX_CONNECT_ATTEMPTS) {
        Serial.println("尝试连接到WiFi...");
        if (WiFi.waitForConnectResult(CONNECT_TIMEOUT) == WL_CONNECTED) {
            Serial.println("已连接到WiFi!");
            Serial.print("IP地址: ");
            Serial.println(WiFi.localIP());
            return true;
        }
        attempts++;
        Serial.println("连接失败。重试中...");
        WiFi.disconnect();
        delay(5000);
    }

    Serial.println("多次尝试后仍无法连接。");
    return false;
}

// 启动配置模式
void WiFiManager::startConfigMode() {
    Serial.println("启动AP进行配置...");
    WiFi.softAP("sxhCameraConfig", "12345678");  // 创建一个名为"sxhCameraConfig"的AP    
    // 设置Web服务器路由
    server.on("/", std::bind(&WiFiManager::handleRoot, this));
    server.on("/config", std::bind(&WiFiManager::handleWiFiConfig, this));
    server.on("/status", std::bind(&WiFiManager::handleStatus, this));
    server.begin();

    while (true) {
        blinkLED(2, 500);  // LED闪烁指示配置模式
        server.handleClient();  // 处理客户端请求
    }
}

// 处理根路径请求
void WiFiManager::handleRoot() {
    String page = "<html><body><h1>WiFi配置</h1>";
    page += "<form action='/config' method='POST'>";
    page += "SSID: <input type='text' name='ssid'><br>";
    page += "密码: <input type='password' name='password'><br>";
    page += "<input type='submit' value='保存'>";
    page += "</form>";
    page += "<br><a href='/status'>检查状态</a>";
    page += "</body></html>";
    server.send(200, "text/html", page);
}

// 处理WiFi配置请求
void WiFiManager::handleWiFiConfig() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    Serial.println("收到新的WiFi配置:");
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    


    if (ssid.length() > 0 && password.length() > 0) {
        saveWiFiConfig(ssid.c_str(), password.c_str());
        server.send(200, "text/html", "WiFi配置已保存。正在重启...");
        delay(1000);
        ESP.restart();
    } else {
        server.send(200, "text/html", "无效的SSID或密码！");
    }
}

// 处理状态请求
void WiFiManager::handleStatus() {
    String status = "当前WiFi IP地址: ";
    status += WiFi.localIP().toString();
    server.send(200, "text/plain", status);
}

// LED闪烁函数
void WiFiManager::blinkLED(int times, int delayTime) {
    for (int i = 0; i < times; ++i) {
        digitalWrite(LED_PIN, HIGH);
        delay(delayTime);
        digitalWrite(LED_PIN, LOW);
        delay(delayTime);
    }
}
