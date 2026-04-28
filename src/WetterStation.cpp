//
// Holgi's Wetterstation
//
// Platform: ESP8266 Wemos D1 Mini 1M SPIFFS

#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#include "config.h"
#include "WetterDebug.h"
#include "TimeClient.h"
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "WebServer.h"
#include "Wifi.h"

TimeClient timeClient(TIMEZONE);

static Ticker tickerForInternalSensorUpdate;
static Ticker tickerForTimeUpdate;
static Ticker tickerForExternalSensorInvalidate;

static volatile bool readyForTimeUpdate = false;

static ICACHE_RAM_ATTR void setReadyForTimeUpdate() {
    readyForTimeUpdate = true;
}

static void updateTime() {
    timeClient.updateTime();
    readyForTimeUpdate = false;
}

void setup() {
    DEBUG_SETUP();

    DEBUG_MSG("compiled: %s %s\n", __DATE__, __TIME__);
    DEBUG_MSG("FW %s\n", FIRMWAREVERSION);
    DEBUG_MSG("SDK: %s\n", ESP.getSdkVersion());

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    setupDisplay();

    if (!setupSensorIndoor()) {
        printAt(6, 0, "ERROR:", false);
        printAt(6, 20, "local", false);
        printAt(6, 30, "Sensor", false);
        printAt(6, 40, "failing!");
        DEBUG_MSG("Couldn't find local sensor!\n");
        while (1) { yield(); }
    }

    printAt(6, 0, "Config...");
    if (shouldStartSetup()) { doSetup(); }

    printAt(6, 10, "WIFI");
    setupWIFI();

    printAt(6, 20, "HTTP...");
    setupWebserver();

    printAt(6, 30, "Time...");
    timeClient.updateTime();

    printAt(6, 40, "433MHz...");
    setupSensorOutdoor();

    tickerForInternalSensorUpdate.attach(MIN_RECEIVE_WAIT_INT, setReadyForInternalSensorUpdate);
    tickerForTimeUpdate.attach(UPDATE_NTP_TIME_INTERVAL, setReadyForTimeUpdate);
    tickerForExternalSensorInvalidate.attach(MAX_RECEIVE_WAIT_EXT / 1000, setExternalSensorInvalid);

    DEBUG_MSG("Ready\n");
}

void loop() {
    drd.loop();
    HTTP.handleClient();

    if (readyForInternalSensorUpdate) { updateInternalSensor(); }
    if (readyForTimeUpdate) { updateTime(); }
    if (isExternalDataAvailable()) { updateExternalSensor(); }

    displayData();
    MDNS.update();

    delay(1000);
    yield();
}
