#include "ConfigStore.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <math.h>

static const char *CONFIG_FILE = "/config.json";

AppConfig ConfigStore::load(const char *defaultNtp, const char *defaultTz, float defaultTempOffsetIndoor) {
    AppConfig cfg{String(defaultNtp), String(defaultTz), defaultTempOffsetIndoor};
    if (!LittleFS.begin()) return cfg;
    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f) { LittleFS.end(); return cfg; }
    JsonDocument doc;
    if (deserializeJson(doc, f) == DeserializationError::Ok) {
        const char *ntp = doc["ntp_server"] | "";
        const char *tz  = doc["timezone"]   | "";
        const float tempOffsetIndoor = doc["temp_offset_indoor"] | defaultTempOffsetIndoor;
        if (strlen(ntp) > 0) cfg.ntpServer     = ntp;
        if (strlen(tz)  > 0) cfg.timezonePosix = tz;
        if (isfinite(tempOffsetIndoor)) cfg.tempOffsetIndoor = tempOffsetIndoor;
    }
    f.close();
    LittleFS.end();
    return cfg;
}

void ConfigStore::save(const AppConfig &config) {
    if (!LittleFS.begin()) {
        LittleFS.format();
        if (!LittleFS.begin()) return;
    }
    File f = LittleFS.open(CONFIG_FILE, "w");
    if (!f) { LittleFS.end(); return; }
    JsonDocument doc;
    doc["ntp_server"] = config.ntpServer;
    doc["timezone"]   = config.timezonePosix;
    doc["temp_offset_indoor"] = config.tempOffsetIndoor;
    serializeJson(doc, f);
    f.close();
    LittleFS.end();
}
