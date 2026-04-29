#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Wifi.h"
#include "WetterDebug.h"
#include "Display.h"
#include "config.h"

Display *Wifi::activeDisplay = nullptr;

Wifi::Wifi() : drd(DRD_TIMEOUT, DRD_ADDRESS) {
}

void Wifi::useDisplay(Display &screen) {
    activeDisplay = &screen;
}

Display &Wifi::display() {
    return *activeDisplay;
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

void Wifi::doSetup(Display &screen) {
    useDisplay(screen);

    DEBUG_MSG("Starting configuration portal.");

    Ticker flasher;
    flasher.attach(0.1, flash);

    wifiManager.setAPCallback(configModeCallback);

    if (!wifiManager.startConfigPortal(HOSTNAME)) {
        DEBUG_MSG("Not connected to WiFi but continuing anyway.");
    } else {
        DEBUG_MSG("Connected to WiFi.");
    }
    ESP.reset();

    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);
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

void Wifi::setup(Display &screen) {
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

void Wifi::loop() {
    drd.loop();
}
