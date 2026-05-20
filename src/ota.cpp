#include "ota.h"
#include <Arduino.h>
#include <HTTPUpdate.h>


String _ota_url = "http://bin.bemfa.com/b/=esp32bin.bin";

//当升级开始时，打印日志
void update_started() {
    Serial.println("OTA:  HTTP update process started");
}

//当升级结束时，打印日志
void update_finished() {
    Serial.println("OTA:  HTTP update process finished");
}

//当升级中，打印日志
void update_progress(int cur, int total) {
    Serial.printf("OTA:  HTTP update process at %d of %d bytes...\n", cur, total);
}

//当升级失败时，打印日志
void update_error(int err) {
    Serial.printf("OTA:  HTTP update fatal error code %d\n", err);
}

/**
 * 固件升级
 * 通过http请求获取远程固件，实现升级
 */
void ota_update() {
    Serial.println("start update");
    if (!WiFi.isConnected()) {
        WiFi.begin();
    }
    int i = 0;
    while(WiFi.status() != WL_CONNECTED) {
        if(i ++ > 5) {
            break;
        }
        delay(1000);
    }
    if ((WiFi.status() == WL_CONNECTED)) {
        Serial.println("start update");
        WiFiClient UpdateClient;

        httpUpdate.onStart(update_started);//当升级开始时
        httpUpdate.onEnd(update_finished);//当升级结束时
        httpUpdate.onProgress(update_progress);//当升级中
        httpUpdate.onError(update_error);//当升级失败时

        t_httpUpdate_return ret = httpUpdate.update(UpdateClient, _ota_url);
        switch (ret) {
        case HTTP_UPDATE_FAILED:      //当升级失败
            Serial.println("[update] Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:  //当无升级
            Serial.println("[update] No update.");
            break;
        case HTTP_UPDATE_OK:         //当升级成功
            Serial.println("[update] Update ok.");
            break;
        }
    } else {
        Serial.println("[update] Update failed due to no wifi.");
    }
}