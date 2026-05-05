//
// Holgi's Wetterstation
//
// Platform: ESP8266 Wemos D1 Mini 4M 1M LittleFS

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#include "config.h"
#include "WeatherDebug.h"
#include "ConfigStore.h"
#include "TimeClient.h"
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "WebServer.h"
#include "Wifi.h"

static TimeClient timeClient;
static Display displayScreen;
static SensorIndoor indoorSensor;
static SensorOutdoor outdoorSensor;
static Wifi wifiController;
static WebServer webServer;

static Ticker indoorMeasurementTicker;
static Ticker outdoorReadingExpiryTicker;

static IRAM_ATTR void markIndoorMeasurementDue() {
    indoorSensor.markMeasurementDue();
}

static IRAM_ATTR void markOutdoorReadingStale() {
    outdoorSensor.markReadingStale();
}

static void configureOta() {
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

    displayScreen.begin();

    if (!indoorSensor.begin()) {
        displayScreen.showSensorFailure();
        DEBUG_MSG("Couldn't find indoor sensor!\n");
        while (1) { yield(); }
    }

    AppConfig appConfig = ConfigStore::load(DEFAULT_NTP_SERVER, DEFAULT_TIMEZONE_POSIX, DEFAULT_TEMP_OFFSET_INDOOR);
    indoorSensor.setTemperatureOffset(appConfig.tempOffsetIndoor);

    displayScreen.showStartupConfig();
    if (wifiController.shouldStartConfigPortal(displayScreen)) { wifiController.startConfigPortal(displayScreen, appConfig); }

    displayScreen.showStartupWifi();
    wifiController.connect(displayScreen, appConfig);

    displayScreen.showStartupHttp();
    webServer.begin(indoorSensor, outdoorSensor);

    displayScreen.showStartupTime();
    timeClient.configure(appConfig.timezonePosix.c_str(), appConfig.ntpServer.c_str());

    displayScreen.showStartupOutdoorSensor();
    outdoorSensor.begin();

    configureOta();

    indoorMeasurementTicker.attach(MIN_RECEIVE_WAIT_INT, markIndoorMeasurementDue);
    outdoorReadingExpiryTicker.attach(MAX_RECEIVE_WAIT_EXT_S, markOutdoorReadingStale);

    DEBUG_MSG("Ready\n");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    wifiController.poll();
    ArduinoOTA.handle();

    if (indoorSensor.isMeasurementDue()) { indoorSensor.refreshMeasurements(); }
    if (outdoorSensor.hasPendingPacket()) { outdoorSensor.refreshMeasurements(); }
    outdoorSensor.applyPendingUpdates();

    unsigned long now = millis();
    if (now - lastDisplayUpdate >= 1000) {
        displayScreen.renderMeasurements(timeClient, indoorSensor, outdoorSensor);
        lastDisplayUpdate = now;
    }

    webServer.handleClient();
    MDNS.update();
}
