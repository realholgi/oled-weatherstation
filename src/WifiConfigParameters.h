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

private:
    String timezoneSelectHtml;
    String languageSelectHtml;
    String tempOffsetIndoorValue;
    String outdoorSensorChannelValue;
    String ventingThresholdValue;

    WiFiManagerParameter timezoneSelectParam;
    WiFiManagerParameter languageSelectParam;
    WiFiManagerParameter timezoneHiddenParam;
    WiFiManagerParameter webLanguageParam;
    WiFiManagerParameter ntpServerParam;
    WiFiManagerParameter tempOffsetIndoorParam;
    WiFiManagerParameter outdoorSensorChannelParam;
    WiFiManagerParameter ventingThresholdParam;

    static String buildTimezoneSelectHtml(const String &currentPosix);
    static String buildLanguageSelectHtml(const String &currentLanguage);
    static String formatFloatValue(float value, uint8_t decimals);
    static String formatIntegerValue(uint8_t value);
};
