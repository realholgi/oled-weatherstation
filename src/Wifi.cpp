#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <stdlib.h>
#include "Wifi.h"
#include "WeatherDebug.h"
#include "Display.h"
#include "ConfigStore.h"
#include "config.h"
#include "Timezones.h"

Display *Wifi::activeDisplay = nullptr;
AppConfig *Wifi::activeConfig = nullptr;
WiFiManagerParameter *Wifi::ntpParam = nullptr;
WiFiManagerParameter *Wifi::tzParam = nullptr;
WiFiManagerParameter *Wifi::tempOffsetIndoorParam = nullptr;

Wifi::Wifi() : doubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS) {
}

void Wifi::setActiveDisplay(Display &screen) {
    activeDisplay = &screen;
}

Display &Wifi::activeDisplayRef() {
    return *activeDisplay;
}

String Wifi::buildTimezoneSelectHtml(const String &currentPosix) {
    String selectMarkup = "<br/><label>Timezone</label>"
                          "<select onchange=\"document.getElementById('timezone').value=this.value\">";
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

String Wifi::formatFloatValue(float value, uint8_t decimals) {
    return String(value, decimals);
}

bool Wifi::shouldStartConfigPortal(Display &screen) {
    setActiveDisplay(screen);

    if (WiFi.SSID() == "") {
        DEBUG_MSG("No stored access-point credentials; initiating configuration portal.");
        activeDisplayRef().showConfigPortalNoCredentials();
        delay(1000);
        return true;
    }

    if (doubleResetDetector.detectDoubleReset()) {
        DEBUG_MSG("Double-reset detected...");
        activeDisplayRef().showConfigPortalReset();
        delay(1000);
        return true;
    }
    return false;
}

void Wifi::startConfigPortal(Display &screen, AppConfig &config) {
    setActiveDisplay(screen);
    activeConfig = &config;

    String selectHtml = buildTimezoneSelectHtml(config.timezonePosix);
    String tempOffsetIndoorValue = formatFloatValue(config.tempOffsetIndoor, 2);
    WiFiManagerParameter tzSelectRaw(selectHtml.c_str());
    WiFiManagerParameter tzHidden("timezone", "", config.timezonePosix.c_str(), 64, "type='hidden'");
    WiFiManagerParameter ntpServer("ntp_server", "NTP Server", config.ntpServer.c_str(), 64);
    WiFiManagerParameter tempOffsetIndoor("temp_offset_indoor", "Indoor Temperature Offset", tempOffsetIndoorValue.c_str(), 16, "type='number' step='0.1'");

    tzParam  = &tzHidden;
    ntpParam = &ntpServer;
    tempOffsetIndoorParam = &tempOffsetIndoor;

    wifiManager.addParameter(&tzSelectRaw);
    wifiManager.addParameter(&tzHidden);
    wifiManager.addParameter(&ntpServer);
    wifiManager.addParameter(&tempOffsetIndoor);

    wifiManager.setSaveParamsCallback(saveConfigParameters);
    wifiManager.setAPCallback(handleConfigPortalStart);

    DEBUG_MSG("Starting configuration portal.");
    Ticker statusLedTicker;
    statusLedTicker.attach(0.1, toggleStatusLed);

    if (!wifiManager.startConfigPortal(HOSTNAME)) {
        DEBUG_MSG("Not connected to WiFi but continuing anyway.");
    } else {
        DEBUG_MSG("Connected to WiFi.");
    }
    ESP.reset();
}

void Wifi::saveConfigParameters() {
    if (!activeConfig || !ntpParam || !tzParam || !tempOffsetIndoorParam) return;
    const char *ntpServerValue = ntpParam->getValue();
    const char *timezoneValue = tzParam->getValue();
    const char *tempOffsetValue = tempOffsetIndoorParam->getValue();
    if (ntpServerValue && strlen(ntpServerValue) > 0) activeConfig->ntpServer     = ntpServerValue;
    if (timezoneValue && strlen(timezoneValue) > 0) activeConfig->timezonePosix = timezoneValue;
    if (tempOffsetValue && strlen(tempOffsetValue) > 0) {
        char *end = nullptr;
        const float parsedIndoorTemperatureOffset = strtof(tempOffsetValue, &end);
        if (end != tempOffsetValue && *end == '\0' && isfinite(parsedIndoorTemperatureOffset)) {
            activeConfig->tempOffsetIndoor = parsedIndoorTemperatureOffset;
        }
    }
    if (!ConfigStore::save(*activeConfig)) {
        DEBUG_MSG("Failed to persist configuration to LittleFS.\n");
    }
}

IRAM_ATTR void Wifi::toggleStatusLed() {
    int ledState = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !ledState);
}

void Wifi::handleConfigPortalStart(WiFiManager *wifiManagerInstance) {
    DEBUG_MSG("Entered config mode");
    DEBUG_MSG(WiFi.softAPIP().toString().c_str());
    DEBUG_MSG(wifiManagerInstance->getConfigPortalSSID().c_str());

    activeDisplayRef().showConfigPortalSsid(wifiManagerInstance->getConfigPortalSSID());
}

void Wifi::connect(Display &screen, AppConfig &config) {
    setActiveDisplay(screen);
    activeConfig = &config;

    String selectHtml = buildTimezoneSelectHtml(config.timezonePosix);
    String tempOffsetIndoorValue = formatFloatValue(config.tempOffsetIndoor, 2);
    WiFiManagerParameter tzSelectRaw(selectHtml.c_str());
    WiFiManagerParameter tzHidden("timezone", "", config.timezonePosix.c_str(), 64, "type='hidden'");
    WiFiManagerParameter ntpServer("ntp_server", "NTP Server", config.ntpServer.c_str(), 64);
    WiFiManagerParameter tempOffsetIndoor("temp_offset_indoor", "Indoor Temperature Offset", tempOffsetIndoorValue.c_str(), 16, "type='number' step='0.1'");

    tzParam  = &tzHidden;
    ntpParam = &ntpServer;
    tempOffsetIndoorParam = &tempOffsetIndoor;

    wifiManager.addParameter(&tzSelectRaw);
    wifiManager.addParameter(&tzHidden);
    wifiManager.addParameter(&ntpServer);
    wifiManager.addParameter(&tempOffsetIndoor);

    wifiManager.setSaveParamsCallback(saveConfigParameters);
    wifiManager.setAPCallback(handleConfigPortalStart);

    String hostname(HOSTNAME);
    WiFi.hostname(hostname);

    wifiManager.autoConnect();

    DEBUG_MSG("Enabling WIFI...\n");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        DEBUG_MSG(".");
        activeDisplayRef().appendWifiConnectionProgress();
    }
    DEBUG_MSG("\n");
    MDNS.begin(HOSTNAME);
}

void Wifi::poll() {
    doubleResetDetector.loop();
}
