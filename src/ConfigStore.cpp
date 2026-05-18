#include "ConfigStore.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <cmath>

static const char *CONFIG_FILE = "/config.json";

AppConfig ConfigStore::load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                            uint8_t defaultOutdoorSensorChannel, const char *defaultWebLanguage, float defaultVentingThreshold) {
    AppConfig config{String(defaultNtpServer), String(defaultTimezonePosix), defaultTempOffsetIndoor,
                     defaultOutdoorSensorChannel, String(defaultWebLanguage), defaultVentingThreshold};
    if (!LittleFS.begin()) return config;
    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (!configFile) { LittleFS.end(); return config; }
    JsonDocument jsonDocument;
    if (deserializeJson(jsonDocument, configFile) == DeserializationError::Ok) {
        const char *ntpServerValue = jsonDocument["ntp_server"] | "";
        const char *timezoneValue  = jsonDocument["timezone_posix"]   | "";
        const char *webLanguageValue = jsonDocument["web_language"] | "";
        const float tempOffsetIndoor = jsonDocument["temp_offset_indoor"] | defaultTempOffsetIndoor;
        const int outdoorSensorChannel = jsonDocument["outdoor_sensor_channel"] | int(defaultOutdoorSensorChannel);
        const float ventingThreshold = jsonDocument["venting_threshold"] | defaultVentingThreshold;
        if (strlen(ntpServerValue) > 0) config.ntpServer     = ntpServerValue;
        if (strlen(timezoneValue)  > 0) config.timezonePosix = timezoneValue;
        if (strcmp(webLanguageValue, "en") == 0 || strcmp(webLanguageValue, "de") == 0) {
            config.webLanguage = webLanguageValue;
        }
        if (std::isfinite(tempOffsetIndoor)) config.tempOffsetIndoor = tempOffsetIndoor;
        if (outdoorSensorChannel >= 1 && outdoorSensorChannel <= 3) {
            config.outdoorSensorChannel = outdoorSensorChannel;
        }
        if (std::isfinite(ventingThreshold) && ventingThreshold > 0.0f) config.ventingThreshold = ventingThreshold;
    }
    configFile.close();
    LittleFS.end();
    return config;
}

bool ConfigStore::save(const AppConfig &config) {
    if (!LittleFS.begin()) return false;
    File configFile = LittleFS.open(CONFIG_FILE, "w");
    if (!configFile) { LittleFS.end(); return false; }
    JsonDocument jsonDocument;
    jsonDocument["ntp_server"] = config.ntpServer;
    jsonDocument["timezone_posix"]   = config.timezonePosix;
    jsonDocument["temp_offset_indoor"] = config.tempOffsetIndoor;
    jsonDocument["outdoor_sensor_channel"] = config.outdoorSensorChannel;
    jsonDocument["web_language"] = config.webLanguage;
    jsonDocument["venting_threshold"] = config.ventingThreshold;
    if (serializeJson(jsonDocument, configFile) == 0) {
        configFile.close();
        LittleFS.end();
        return false;
    }
    configFile.close();
    LittleFS.end();
    return true;
}
