//
// Holgi's Wetterstation
//
// Platform: ESP8266 Wemos D1 Mini 4M 1M LittleFS

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#include "config.h"
#include "WetterDebug.h"
#include "ConfigStore.h"
#include "TimeClient.h"
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "WebServer.h"
#include "Wifi.h"

static TimeClient timeClient;
static Display display;
static SensorIndoor sensorIndoor;
static SensorOutdoor sensorOutdoor;
static Wifi wifi;
static WebServer webServer;

static Ticker tickerForInternalSensorUpdate;
static Ticker tickerForExternalSensorInvalidate;

static IRAM_ATTR void setReadyForIndoorSensorUpdate() {
    sensorIndoor.setReadyForUpdate();
}

static IRAM_ATTR void invalidateOutdoorSensor() {
    sensorOutdoor.invalidate();
}

static void setupOta() {
    ArduinoOTA.setHostname(HOSTNAME);

#ifdef OTA_PASSWORD
    ArduinoOTA.setPassword(OTA_PASSWORD);
#endif

    ArduinoOTA.onStart([]() {
        DEBUG_MSG("OTA update started\n");
    });
    ArduinoOTA.onEnd([]() {
        DEBUG_MSG("OTA update finished\n");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_MSG("OTA error %u\n", error);
    });

    ArduinoOTA.begin();
    DEBUG_MSG("OTA ready at %s.local\n", HOSTNAME);
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

    AppConfig config = ConfigStore::load(DEFAULT_NTP_SERVER, DEFAULT_TIMEZONE_POSIX, DEFAULT_TEMP_OFFSET_INDOOR);
    sensorIndoor.setTemperatureOffset(config.tempOffsetIndoor);

    display.showStartupConfig();
    if (wifi.shouldStartSetup(display)) { wifi.doSetup(display, config); }

    display.showStartupWifi();
    wifi.setup(display, config);

    display.showStartupHttp();
    webServer.setup(sensorIndoor, sensorOutdoor);

    display.showStartupTime();
    timeClient.configure(config.timezonePosix.c_str(), config.ntpServer.c_str());

    display.showStartupOutdoorSensor();
    sensorOutdoor.setup();

    setupOta();

    tickerForInternalSensorUpdate.attach(MIN_RECEIVE_WAIT_INT, setReadyForIndoorSensorUpdate);
    tickerForExternalSensorInvalidate.attach(MAX_RECEIVE_WAIT_EXT_S, invalidateOutdoorSensor);

    DEBUG_MSG("Ready\n");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    wifi.loop();
    ArduinoOTA.handle();

    if (sensorIndoor.isReadyForUpdate()) { sensorIndoor.update(); }
    if (sensorOutdoor.isDataAvailable()) { sensorOutdoor.update(); }

    unsigned long now = millis();
    if (now - lastDisplayUpdate >= 1000) {
        display.renderData(timeClient, sensorIndoor, sensorOutdoor);
        lastDisplayUpdate = now;
    }

    webServer.handleClient();
    MDNS.update();
}
