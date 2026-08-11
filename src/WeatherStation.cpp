//
// Holgi's Wetterstation
//
// Platform: ESP8266 Wemos D1 Mini 4M 1M LittleFS

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>

#include "BuildInfoGenerated.h"
#include "config.h"
#include "WeatherDebug.h"
#include "ConfigStore.h"
#include "TimeClient.h"
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "StartupRetryPolicy.h"
#include "MqttPublishPolicy.h"
#include "WebServer.h"
#include "Wifi.h"

static TimeClient timeClient;
static Display displayScreen;
static SensorIndoor indoorSensor;
static SensorOutdoor outdoorSensor;
static Wifi wifiController;
static WebServer webServer;
static bool otaReady = false;
static AppConfig appConfig;

static Ticker indoorMeasurementTicker;
static Ticker outdoorReadingExpiryTicker;

// Ticker callback — flag-only: must not read sensors, log, allocate, or perform display/network work.
// loop() consumes the flag via indoorSensor.isMeasurementDue() / refreshMeasurements().
static IRAM_ATTR void markIndoorMeasurementDue() {
    indoorSensor.markMeasurementDue();
}

// Ticker callback — flag-only: must not read sensors, log, allocate, or perform display/network work.
// loop() consumes the flag via outdoorSensor.applyPendingUpdates().
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
        (void) error; // only used when DEBUG_ESP_PORT is defined
        DEBUG_MSG("OTA error %u\n", error);
    });

    ArduinoOTA.begin();
    otaReady = true;
    DEBUG_MSG("OTA ready at %s.local\n", HOSTNAME);
}

void setup() {
    DEBUG_SETUP();

    DEBUG_MSG("compiled: %s %s\n", __DATE__, __TIME__);
    DEBUG_MSG("FW %s\n", VERSION_STRING);
    DEBUG_MSG("SDK: %s\n", ESP.getSdkVersion());

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    displayScreen.begin();

    bool indoorSensorReady = false;
    for (uint8_t attempt = 1; attempt <= StartupRetryPolicy::maxAttempts(); ++attempt) {
        if (indoorSensor.begin()) {
            indoorSensorReady = true;
            break;
        }

        DEBUG_MSG("Indoor sensor init failed (%u/%u)\n", attempt, StartupRetryPolicy::maxAttempts());
        if (StartupRetryPolicy::shouldRetryAfterAttempt(attempt)) {
            delay(StartupRetryPolicy::retryDelayMs());
        }
    }

    if (!indoorSensorReady) {
        displayScreen.showSensorFailure();
        DEBUG_MSG("Couldn't find indoor sensor!\n");
        while (1) { yield(); }
    }

    ConfigLoadResult configLoad = ConfigStore::load(DEFAULT_NTP_SERVER, DEFAULT_TIMEZONE_POSIX, DEFAULT_TEMP_OFFSET_INDOOR,
                                                    DEFAULT_OUTDOOR_SENSOR_CHANNEL, DEFAULT_WEB_LANGUAGE, DEFAULT_VENTING_THRESHOLD,
                                                    DEFAULT_MQTT_HOST, DEFAULT_MQTT_PORT, DEFAULT_MQTT_USERNAME, DEFAULT_MQTT_PASSWORD);
    appConfig = configLoad.config;
    DEBUG_MSG("Config load: %s (%s)\n",
              ConfigLoadStatus::statusLabel(configLoad.outcome.status),
              ConfigLoadStatus::reasonLabel(configLoad.outcome.reason));
    indoorSensor.setTemperatureOffset(appConfig.tempOffsetIndoor);

    displayScreen.showStartupConfig();
    if (configLoad.outcome.usedDefaults) {
        displayScreen.showStartupConfigDefault();
        delay(1000);
    }
    if (wifiController.shouldStartConfigPortal(displayScreen)) { wifiController.startConfigPortal(displayScreen, appConfig); }

    displayScreen.showStartupWifi();
    const bool wifiConnected = wifiController.connect(displayScreen, appConfig);
    wifiController.configureMqtt(appConfig);
    displayScreen.setVentingThreshold(appConfig.ventingThreshold);  // after connect() so portal changes are picked up
    if (wifiConnected) {
        displayScreen.showStartupHttp();
        webServer.begin(indoorSensor, outdoorSensor, timeClient, wifiController.isMdnsReady(), appConfig.webLanguage, appConfig.ventingThreshold);

        displayScreen.showStartupTime();
        timeClient.configure(appConfig.timezonePosix.c_str(), appConfig.ntpServer.c_str());
    } else {
        displayScreen.showWifiFailure();
    }

    displayScreen.showStartupOutdoorSensor();
    outdoorSensor.begin(appConfig.outdoorSensorChannel);

    if (wifiConnected) {
        configureOta();
    }

    indoorMeasurementTicker.attach(MIN_RECEIVE_WAIT_INT, markIndoorMeasurementDue);
    outdoorReadingExpiryTicker.attach(MAX_RECEIVE_WAIT_EXT_S, markOutdoorReadingStale);

    DEBUG_MSG("Ready\n");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    static uint32_t lastMqttPublishAtMillis = 0;
    const bool mqttJustConnected = wifiController.poll();
    if (otaReady) {
        ArduinoOTA.handle();
    }
    const bool indoorUpdated = indoorSensor.isMeasurementDue();
    if (indoorUpdated) indoorSensor.refreshMeasurements();
    const bool outdoorUpdated = outdoorSensor.hasPendingPacket();
    if (outdoorUpdated) outdoorSensor.refreshMeasurements();
    outdoorSensor.applyPendingUpdates();

    const uint32_t now = millis();
    if (MqttPublishPolicy::shouldPublish(
            wifiController.isMqttConnected(), mqttJustConnected, now, lastMqttPublishAtMillis)) {
        const DataJsonPayload::IndoorInputs indoor{
            indoorSensor.isValid(), indoorSensor.temperature(), indoorSensor.humidity(),
            indoorSensor.absoluteHumidity(), indoorSensor.dewPoint()};
        const DataJsonPayload::OutdoorInputs outdoor{
            outdoorSensor.isValid(), outdoorSensor.temperature(), outdoorSensor.humidity(),
            outdoorSensor.absoluteHumidity(), outdoorSensor.batteryStatus(), outdoorSensor.secondsSinceLastPacket()};
        wifiController.publishSensorStates(DataJsonPayload::build(
            indoor, outdoor, appConfig.ventingThreshold, timeClient.isTimeSet()));
        lastMqttPublishAtMillis = now;
    }

    if (now - lastDisplayUpdate >= 1000) {
        displayScreen.renderMeasurements(timeClient, indoorSensor, outdoorSensor);
        lastDisplayUpdate = now;
    }

    webServer.handleClient();
    if (wifiController.isMdnsReady()) {
        MDNS.update();
    }
}
