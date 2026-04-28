#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Wifi.h"
#include "WetterDebug.h"
#include "Display.h"
#include "config.h"

namespace Wifi {

WiFiManager wifiManager;
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

static bool shouldSaveConfig = false;
static bool initialConfig = false;
static Display *activeDisplay = nullptr;

static void useDisplay(Display &screen) {
    activeDisplay = &screen;
}

static Display &display() {
    return *activeDisplay;
}

bool shouldStartSetup(Display &screen) {
    useDisplay(screen);

    if (WiFi.SSID() == "") {
        DEBUG_MSG("No stored access-point credentials; initiating configuration portal.");
        display().showConfigPortalNoCredentials();
        delay(1000);
        initialConfig = true;
        return true;
    }

    if (drd.detectDoubleReset()) {
        DEBUG_MSG("Double-reset detected...");
        display().showConfigPortalReset();
        delay(1000);
        initialConfig = true;
        return true;
    }
    return false;
}

void doSetup(Display &screen) {
    useDisplay(screen);

    if (initialConfig) {
        DEBUG_MSG("Starting configuration portal.");

        Ticker flasher;
        flasher.attach(0.1, flash);

        wifiManager.setSaveConfigCallback(saveConfigCallback);
        wifiManager.setAPCallback(configModeCallback);

        String hostname = "ESP" + String(ESP.getChipId(), HEX);
        DEBUG_MSG("Hostname:  ");
        DEBUG_MSG("%s\n", hostname.c_str());

        if (!wifiManager.startConfigPortal(HOSTNAME)) {
            DEBUG_MSG("Not connected to WiFi but continuing anyway.");
        } else {
            DEBUG_MSG("Connected to WiFi.");
        }
        ESP.reset();
    }
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);
}

void saveConfigCallback() {
    DEBUG_MSG("Should save config");
    shouldSaveConfig = true;
}

ICACHE_RAM_ATTR void flash() {
    int state = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !state);
}

void configModeCallback(WiFiManager *myWiFiManager) {
    DEBUG_MSG("Entered config mode");
    DEBUG_MSG(WiFi.softAPIP().toString().c_str());
    DEBUG_MSG(myWiFiManager->getConfigPortalSSID().c_str());

    display().showConfigPortalSsid(myWiFiManager->getConfigPortalSSID());
}

void setup(Display &screen) {
    useDisplay(screen);

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

}
