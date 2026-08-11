#pragma once

#include <WiFiManager.h>
#include "ConfigStore.h"

class WifiConfigParameters {
public:
    explicit WifiConfigParameters(const AppConfig &config);

    WiFiManagerParameter &timezoneSelect();
    WiFiManagerParameter &languageSelect();
    WiFiManagerParameter &timezoneHidden();
    WiFiManagerParameter &webLanguage();
    WiFiManagerParameter &ntpServer();
    WiFiManagerParameter &tempOffsetIndoor();
    WiFiManagerParameter &outdoorSensorChannel();
    WiFiManagerParameter &ventingThreshold();
    WiFiManagerParameter &mqttHost();
    WiFiManagerParameter &mqttPort();
    WiFiManagerParameter &mqttUsername();
    WiFiManagerParameter &mqttPassword();

private:
    String timezoneSelectHtml;
    String languageSelectHtml;
    String tempOffsetIndoorValue;
    String outdoorSensorChannelValue;
    String ventingThresholdValue;
    String mqttPortValue;

    WiFiManagerParameter timezoneSelectParam;
    WiFiManagerParameter languageSelectParam;
    WiFiManagerParameter timezoneHiddenParam;
    WiFiManagerParameter webLanguageParam;
    WiFiManagerParameter ntpServerParam;
    WiFiManagerParameter tempOffsetIndoorParam;
    WiFiManagerParameter outdoorSensorChannelParam;
    WiFiManagerParameter ventingThresholdParam;
    WiFiManagerParameter mqttHostParam;
    WiFiManagerParameter mqttPortParam;
    WiFiManagerParameter mqttUsernameParam;
    WiFiManagerParameter mqttPasswordParam;

    static String formatFloatValue(float value, uint8_t decimals);
    static String formatIntegerValue(uint8_t value);
    static String formatMqttPortValue(uint16_t value);
};
