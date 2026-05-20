#include "chaoxing.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <_preference.h>

// 将课程开始节次映射到J-Calendar槽位
// 槽位: 0=第1-2节, 1=第3-4节, 2=第5-6节, 3=第7-8节, 4=第9-10节
static int beginToSlot(int begin) {
    if (begin <= 2) return 0;
    if (begin <= 4) return 1;
    if (begin <= 6) return 2;
    if (begin <= 8) return 3;
    return 4;
}

// 从学习通获取课表并保存到Preferences
bool chaoxing_fetch_and_save() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String ck = pref.getString(PREF_CHAOXING_CK);
    pref.end();

    if (ck.length() == 0) {
        Serial.println("[ChaoXing] No cookie, skip.");
        return false;
    }

    Serial.println("[ChaoXing] Fetching schedule...");

    HTTPClient http;
    WiFiClientSecure wifiClient;
    wifiClient.setInsecure();

    // 第一次请求：获取当前周次
    String url = "https://kb.chaoxing.com/pc/curriculum/getMyLessons";
    http.begin(wifiClient, url);
    http.addHeader("Cookie", ck);
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    http.addHeader("Referer", "https://kb.chaoxing.com/");

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[ChaoXing] HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[ChaoXing] JSON parse error: %s\n", err.c_str());
        return false;
    }

    int currentWeek = doc["data"]["curriculum"]["currentWeek"] | 1;
    Serial.printf("[ChaoXing] Current week: %d\n", currentWeek);

    // 第二次请求：获取指定周次的课表
    url = "https://kb.chaoxing.com/pc/curriculum/getMyLessons?week=" + String(currentWeek);
    http.begin(wifiClient, url);
    http.addHeader("Cookie", ck);
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    http.addHeader("Referer", "https://kb.chaoxing.com/");

    httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[ChaoXing] HTTP error (week %d): %d\n", currentWeek, httpCode);
        http.end();
        return false;
    }

    payload = http.getString();
    http.end();

    err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[ChaoXing] JSON parse error: %s\n", err.c_str());
        return false;
    }

    // 解析课程数据
    // schedule[dayOfWeek][slot] = 课程名
    // dayOfWeek: 1-7 (周一到周日)
    // slot: 0-4 (上午2节+下午2节+晚上1节)
    String schedule[8][5]; // [1-7][0-4]

    JsonArray lessons = doc["data"]["lessonArray"];
    for (JsonObject lesson : lessons) {
        int dayOfWeek = lesson["dayOfWeek"] | 0;
        int beginNumber = lesson["beginNumber"] | 0;
        const char* name = lesson["name"] | "";
        const char* weeksStr = lesson["weeks"] | "";

        if (dayOfWeek < 1 || dayOfWeek > 7 || beginNumber < 1) continue;

        // 检查当前周是否在课程周次内
        bool inWeek = false;
        String weeks(weeksStr);
        int searchStart = 0;
        while (searchStart < (int)weeks.length()) {
            int commaPos = weeks.indexOf(',', searchStart);
            String w;
            if (commaPos < 0) {
                w = weeks.substring(searchStart);
                searchStart = weeks.length();
            } else {
                w = weeks.substring(searchStart, commaPos);
                searchStart = commaPos + 1;
            }
            w.trim();
            if (w.toInt() == currentWeek) {
                inWeek = true;
                break;
            }
        }

        if (!inWeek) continue;

        int slot = beginToSlot(beginNumber);
        schedule[dayOfWeek][slot] = String(name);
    }

    // 构建J-Calendar格式字符串
    // 格式: 221;星期,槽0,槽1,槽2,槽3,槽4;...
    String result = "221";
    const char* weekdays[] = {"", "一", "二", "三", "四", "五", "六", "日"};

    for (int day = 1; day <= 7; day++) {
        result += ";";
        result += weekdays[day];
        for (int slot = 0; slot < 5; slot++) {
            result += ",";
            result += schedule[day][slot];
        }
    }
    result += ";";

    Serial.printf("[ChaoXing] Schedule: %s\n", result.c_str());

    // 保存到Preferences
    pref.begin(PREF_NAMESPACE);
    pref.putString(PREF_STUDY_SCHEDULE, result);
    pref.end();

    Serial.println("[ChaoXing] Schedule saved.");
    return true;
}
