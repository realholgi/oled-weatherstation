#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Wifi.h"
#include "WetterDebug.h"
#include "Display.h"
#include "ConfigStore.h"
#include "config.h"
#include "Timezones.h"

Display *Wifi::activeDisplay = nullptr;
AppConfig *Wifi::activeConfig = nullptr;
WiFiManagerParameter *Wifi::ntpParam = nullptr;
WiFiManagerParameter *Wifi::tzParam = nullptr;

Wifi::Wifi() : drd(DRD_TIMEOUT, DRD_ADDRESS) {
}

void Wifi::useDisplay(Display &screen) {
    activeDisplay = &screen;
}

Display &Wifi::display() {
    return *activeDisplay;
}

String Wifi::buildTimezoneSelectHtml(const String &currentPosix) {
    String html = "<br/><label>Timezone</label>"
                  "<select onchange=\"document.getElementById('timezone').value=this.value\">";
    for (size_t i = 0; i < TZ_COUNT; i++) {
        html += "<option value='";
        html += TIMEZONES[i].posix;
        html += "'";
        if (currentPosix == TIMEZONES[i].posix) html += " selected";
        html += ">";
        html += TIMEZONES[i].name;
        html += "</option>";
    }
    html += "</select>";
    return html;
}

bool Wifi::shouldStartSetup(Display &screen) {
    useDisplay(screen);

    if (WiFi.SSID() == "") {
        DEBUG_MSG("No stored access-point credentials; initiating configuration portal.");
        display().showConfigPortalNoCredentials();
        delay(1000);
        return true;
    }

    if (drd.detectDoubleReset()) {
        DEBUG_MSG("Double-reset detected...");
        display().showConfigPortalReset();
        delay(1000);
        return true;
    }
    return false;
}

void Wifi::doSetup(Display &screen, AppConfig &config) {
    useDisplay(screen);
    activeConfig = &config;

    String selectHtml = buildTimezoneSelectHtml(config.timezonePosix);
    WiFiManagerParameter tzSelectRaw(selectHtml.c_str());
    WiFiManagerParameter tzHidden("timezone", "", config.timezonePosix.c_str(), 64, "type='hidden'");
    WiFiManagerParameter ntpServer("ntp_server", "NTP Server", config.ntpServer.c_str(), 64);

    tzParam  = &tzHidden;
    ntpParam = &ntpServer;

    wifiManager.addParameter(&tzSelectRaw);
    wifiManager.addParameter(&tzHidden);
    wifiManager.addParameter(&ntpServer);

    wifiManager.setSaveParamsCallback(saveParamsCallback);
    wifiManager.setAPCallback(configModeCallback);

    DEBUG_MSG("Starting configuration portal.");
    Ticker flasher;
    flasher.attach(0.1, flash);

    if (!wifiManager.startConfigPortal(HOSTNAME)) {
        DEBUG_MSG("Not connected to WiFi but continuing anyway.");
    } else {
        DEBUG_MSG("Connected to WiFi.");
    }
    ESP.reset();
}

void Wifi::saveParamsCallback() {
    if (!activeConfig || !ntpParam || !tzParam) return;
    const char *ntp = ntpParam->getValue();
    const char *tz  = tzParam->getValue();
    if (ntp && strlen(ntp) > 0) activeConfig->ntpServer     = ntp;
    if (tz  && strlen(tz)  > 0) activeConfig->timezonePosix = tz;
    ConfigStore::save(*activeConfig);
}

IRAM_ATTR void Wifi::flash() {
    int state = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !state);
}

void Wifi::configModeCallback(WiFiManager *myWiFiManager) {
    DEBUG_MSG("Entered config mode");
    DEBUG_MSG(WiFi.softAPIP().toString().c_str());
    DEBUG_MSG(myWiFiManager->getConfigPortalSSID().c_str());

    display().showConfigPortalSsid(myWiFiManager->getConfigPortalSSID());
}

void Wifi::setup(Display &screen, AppConfig &config) {
    useDisplay(screen);
    activeConfig = &config;

    String selectHtml = buildTimezoneSelectHtml(config.timezonePosix);
    WiFiManagerParameter tzSelectRaw(selectHtml.c_str());
    WiFiManagerParameter tzHidden("timezone", "", config.timezonePosix.c_str(), 64, "type='hidden'");
    WiFiManagerParameter ntpServer("ntp_server", "NTP Server", config.ntpServer.c_str(), 64);

    tzParam  = &tzHidden;
    ntpParam = &ntpServer;

    wifiManager.addParameter(&tzSelectRaw);
    wifiManager.addParameter(&tzHidden);
    wifiManager.addParameter(&ntpServer);

    wifiManager.setSaveParamsCallback(saveParamsCallback);
    wifiManager.setAPCallback(configModeCallback);

    String hostname(HOSTNAME);
    WiFi.hostname(hostname);

    wifiManager.autoConnect();

    DEBUG_MSG("Enabling WIFI...\n");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        DEBUG_MSG(".");
        display().appendWifiProgress();
    }
    DEBUG_MSG("\n");
    MDNS.begin(HOSTNAME);
}

void Wifi::loop() {
    drd.loop();
}
