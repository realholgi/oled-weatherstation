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

static TimeClient timeClient(TIMEZONE);
static Display display;
static SensorIndoor sensorIndoor;
static SensorOutdoor sensorOutdoor;
static Wifi wifi;
static WebServer webServer;

static Ticker tickerForInternalSensorUpdate;
static Ticker tickerForTimeUpdate;
static Ticker tickerForExternalSensorInvalidate;

static volatile bool readyForTimeUpdate = false;

static ICACHE_RAM_ATTR void setReadyForIndoorSensorUpdate() {
    sensorIndoor.setReadyForUpdate();
}

static ICACHE_RAM_ATTR void setReadyForTimeUpdate() {
    readyForTimeUpdate = true;
}

static ICACHE_RAM_ATTR void invalidateOutdoorSensor() {
    sensorOutdoor.invalidate();
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

    display.setup();

    if (!sensorIndoor.setup()) {
        display.showSensorFailure();
        DEBUG_MSG("Couldn't find indoor sensor!\n");
        while (1) { yield(); }
    }

    display.showStartupConfig();
    if (wifi.shouldStartSetup(display)) { wifi.doSetup(display); }

    display.showStartupWifi();
    wifi.setup(display);

    display.showStartupHttp();
    webServer.setup();

    display.showStartupTime();
    timeClient.updateTime();

    display.showStartupOutdoorSensor();
    sensorOutdoor.setup();

    tickerForInternalSensorUpdate.attach(MIN_RECEIVE_WAIT_INT, setReadyForIndoorSensorUpdate);
    tickerForTimeUpdate.attach(UPDATE_NTP_TIME_INTERVAL, setReadyForTimeUpdate);
    tickerForExternalSensorInvalidate.attach(MAX_RECEIVE_WAIT_EXT / 1000, invalidateOutdoorSensor);

    DEBUG_MSG("Ready\n");
}

void loop() {
    wifi.loop();
    webServer.handleClient(sensorIndoor, sensorOutdoor);

    if (sensorIndoor.isReadyForUpdate()) { sensorIndoor.update(); }
    if (readyForTimeUpdate) { updateTime(); }
    if (sensorOutdoor.isDataAvailable()) { sensorOutdoor.update(); }

    display.renderData(timeClient, sensorIndoor, sensorOutdoor);
    MDNS.update();

    delay(1000);
    yield();
}
