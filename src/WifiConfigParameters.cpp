#include "WifiConfigParameters.h"

#include "Timezones.h"

WifiConfigParameters::WifiConfigParameters(const AppConfig &config)
    : timezoneSelectHtml(buildTimezoneSelectHtml(config.timezonePosix)),
      languageSelectHtml(buildLanguageSelectHtml(config.webLanguage)),
      tempOffsetIndoorValue(formatFloatValue(config.tempOffsetIndoor, 2)),
      outdoorSensorChannelValue(formatIntegerValue(config.outdoorSensorChannel)),
      ventingThresholdValue(formatFloatValue(config.ventingThreshold, 1)),
      timezoneSelectParam(timezoneSelectHtml.c_str()),
      languageSelectParam(languageSelectHtml.c_str()),
      timezoneHiddenParam("timezone_posix", "", config.timezonePosix.c_str(), 64, "type='hidden'"),
      webLanguageParam("web_language", "", config.webLanguage.c_str(), 8, "type='hidden'"),
      ntpServerParam("ntp_server", "NTP Server", config.ntpServer.c_str(), 64),
      tempOffsetIndoorParam("temp_offset_indoor", "Indoor Temperature Offset", tempOffsetIndoorValue.c_str(), 16, "type='number' step='0.1'"),
      outdoorSensorChannelParam("outdoor_sensor_channel", "Outdoor Sensor Channel",
                                outdoorSensorChannelValue.c_str(), 4, "type='number' min='1' max='3' step='1'"),
      ventingThresholdParam("venting_threshold", "Venting Threshold (g/m\xc2\xb3)",
                            ventingThresholdValue.c_str(), 8, "type='number' min='0.5' max='10' step='0.5'") {
}

WiFiManagerParameter &WifiConfigParameters::timezoneSelect() {
    return timezoneSelectParam;
}

WiFiManagerParameter &WifiConfigParameters::languageSelect() {
    return languageSelectParam;
}

WiFiManagerParameter &WifiConfigParameters::timezoneHidden() {
    return timezoneHiddenParam;
}

WiFiManagerParameter &WifiConfigParameters::webLanguage() {
    return webLanguageParam;
}

WiFiManagerParameter &WifiConfigParameters::ntpServer() {
    return ntpServerParam;
}

WiFiManagerParameter &WifiConfigParameters::tempOffsetIndoor() {
    return tempOffsetIndoorParam;
}

WiFiManagerParameter &WifiConfigParameters::outdoorSensorChannel() {
    return outdoorSensorChannelParam;
}

WiFiManagerParameter &WifiConfigParameters::ventingThreshold() {
    return ventingThresholdParam;
}

String WifiConfigParameters::buildTimezoneSelectHtml(const String &currentPosix) {
    String selectMarkup = "<br/><label>Timezone</label>"
                          "<select onchange=\"document.getElementById('timezone_posix').value=this.value\">";
    for (size_t i = 0; i < TZ_COUNT; i++) {
        selectMarkup += "<option value='";
        selectMarkup += TIMEZONES[i].posix;
        selectMarkup += "'";
        if (currentPosix == TIMEZONES[i].posix) selectMarkup += " selected";
        selectMarkup += ">";
        selectMarkup += TIMEZONES[i].name;
        selectMarkup += "</option>";
    }
    selectMarkup += "</select>";
    return selectMarkup;
}

String WifiConfigParameters::buildLanguageSelectHtml(const String &currentLanguage) {
    const bool isEnglish = currentLanguage == "en";
    String selectMarkup = "<br/><label>Webpage Language</label>"
                          "<select onchange=\"document.getElementById('web_language').value=this.value\">";
    selectMarkup += "<option value='de'";
    if (!isEnglish) selectMarkup += " selected";
    selectMarkup += ">Deutsch</option>";
    selectMarkup += "<option value='en'";
    if (isEnglish) selectMarkup += " selected";
    selectMarkup += ">English</option>";
    selectMarkup += "</select>";
    return selectMarkup;
}

String WifiConfigParameters::formatFloatValue(float value, uint8_t decimals) {
    return String(value, decimals);
}

String WifiConfigParameters::formatIntegerValue(uint8_t value) {
    return String(value);
}
