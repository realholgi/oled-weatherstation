#include "ConfigStore.h"
#include "ConfigJsonParser.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

static const char *CONFIG_FILE = "/config.json";

namespace {
ConfigLoadResult resultFromDefaults(const char *defaultNtpServer, const char *defaultTimezonePosix,
                                    float defaultTempOffsetIndoor, uint8_t defaultOutdoorSensorChannel,
                                    const char *defaultWebLanguage, float defaultVentingThreshold,
                                    ConfigLoadStatus::Outcome outcome) {
    AppConfig config{String(defaultNtpServer), String(defaultTimezonePosix), defaultTempOffsetIndoor,
                     defaultOutdoorSensorChannel, String(defaultWebLanguage), defaultVentingThreshold};
    return {config, outcome};
}

AppConfig appConfigFromParsedValues(const ConfigJsonParser::Values &values) {
    return {String(values.ntpServer.c_str()),
            String(values.timezonePosix.c_str()),
            values.tempOffsetIndoor,
            values.outdoorSensorChannel,
            String(values.webLanguage.c_str()),
            values.ventingThreshold};
}
}

ConfigLoadResult ConfigStore::load(const char *defaultNtpServer, const char *defaultTimezonePosix, float defaultTempOffsetIndoor,
                                   uint8_t defaultOutdoorSensorChannel, const char *defaultWebLanguage, float defaultVentingThreshold) {
    if (!LittleFS.begin()) {
        return resultFromDefaults(defaultNtpServer, defaultTimezonePosix, defaultTempOffsetIndoor,
                                  defaultOutdoorSensorChannel, defaultWebLanguage, defaultVentingThreshold,
                                  ConfigLoadStatus::mountFailed());
    }

    File configFile = LittleFS.open(CONFIG_FILE, "r");
    if (!configFile) {
        LittleFS.end();
        return resultFromDefaults(defaultNtpServer, defaultTimezonePosix, defaultTempOffsetIndoor,
                                  defaultOutdoorSensorChannel, defaultWebLanguage, defaultVentingThreshold,
                                  ConfigLoadStatus::missingFile());
    }

    String configContent;
    while (configFile.available()) {
        configContent += static_cast<char>(configFile.read());
    }
    configFile.close();
    LittleFS.end();

    const ConfigJsonParser::Defaults defaults{defaultNtpServer, defaultTimezonePosix, defaultTempOffsetIndoor,
                                              defaultOutdoorSensorChannel, defaultWebLanguage, defaultVentingThreshold};
    const ConfigJsonParser::Result parsed = ConfigJsonParser::parse(configContent.c_str(), defaults);
    return {appConfigFromParsedValues(parsed.values), parsed.outcome};
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
