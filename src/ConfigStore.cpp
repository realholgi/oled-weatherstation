#include "ConfigStore.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <math.h>

static const char *CONFIG_FILE = "/config.json";

AppConfig ConfigStore::load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor) {
    AppConfig config{String(defaultNtpServer), String(defaultTimezonePosix), defaultTempOffsetIndoor};
    if (!LittleFS.begin()) return config;
    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (!configFile) { LittleFS.end(); return config; }
    JsonDocument jsonDocument;
    if (deserializeJson(jsonDocument, configFile) == DeserializationError::Ok) {
        const char *ntpServerValue = jsonDocument["ntp_server"] | "";
        const char *timezoneValue  = jsonDocument["timezone"]   | "";
        const float tempOffsetIndoor = jsonDocument["temp_offset_indoor"] | defaultTempOffsetIndoor;
        if (strlen(ntpServerValue) > 0) config.ntpServer     = ntpServerValue;
        if (strlen(timezoneValue)  > 0) config.timezonePosix = timezoneValue;
        if (isfinite(tempOffsetIndoor)) config.tempOffsetIndoor = tempOffsetIndoor;
    }
    configFile.close();
    LittleFS.end();
    return config;
}

void ConfigStore::save(const AppConfig &config) {
    if (!LittleFS.begin()) {
        LittleFS.format();
        if (!LittleFS.begin()) return;
    }
    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) { LittleFS.end(); return; }
    JsonDocument jsonDocument;
    jsonDocument["ntp_server"] = config.ntpServer;
    jsonDocument["timezone"]   = config.timezonePosix;
    jsonDocument["temp_offset_indoor"] = config.tempOffsetIndoor;
    serializeJson(jsonDocument, configFile);
    configFile.close();
    LittleFS.end();
}
