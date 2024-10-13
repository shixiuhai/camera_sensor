// sd_config.h
#ifndef SD_CONFIG_H
#define SD_CONFIG_H

#include "FS.h"
#include "SD.h"
#include "SPI.h"

// 初始化 SD 卡
bool initSDCard() {
    Serial.println("Initializing SD card...");
    if(!SD.begin(21)){  // 确保21为CS引脚
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();
    // Determine if the type of SD card is available
    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    Serial.println("SD卡初始化成功");
    return true;
}

#endif // SD_CONFIG_H
