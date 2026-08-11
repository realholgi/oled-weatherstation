#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Wifi.h"
#include "WeatherDebug.h"
#include "Display.h"
#include "ConfigStore.h"
#include "WifiConfigFields.h"
#include "config.h"
#include "MqttDiscovery.h"
#include "BuildInfoGenerated.h"

static const char *PORTAL_TITLE = "Weather Station Setup";

Display *Wifi::activeDisplay = nullptr;
AppConfig *Wifi::activeConfig = nullptr;
WifiConfigParameters *Wifi::activeConfigParameters = nullptr;

Wifi::Wifi() : doubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS), mqttClient(mqttTransport) {
}

void Wifi::setActiveDisplay(Display &screen) {
    activeDisplay = &screen;
}

Display &Wifi::activeDisplayRef() {
    return *activeDisplay;
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

    prepareConfigPortalParameters(config);

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

void Wifi::prepareConfigPortalParameters(AppConfig &config) {
    configParameters.reset(new WifiConfigParameters(config));
    activeConfigParameters = configParameters.get();

    wifiManager.addParameter(&activeConfigParameters->timezoneSelect());
    wifiManager.addParameter(&activeConfigParameters->languageSelect());
    wifiManager.addParameter(&activeConfigParameters->timezoneHidden());
    wifiManager.addParameter(&activeConfigParameters->webLanguage());
    wifiManager.addParameter(&activeConfigParameters->ntpServer());
    wifiManager.addParameter(&activeConfigParameters->tempOffsetIndoor());
    wifiManager.addParameter(&activeConfigParameters->outdoorSensorChannel());
    wifiManager.addParameter(&activeConfigParameters->ventingThreshold());
    wifiManager.addParameter(&activeConfigParameters->mqttHost());
    wifiManager.addParameter(&activeConfigParameters->mqttPort());
    wifiManager.addParameter(&activeConfigParameters->mqttUsername());
    wifiManager.addParameter(&activeConfigParameters->mqttPassword());

    wifiManager.setSaveParamsCallback(saveConfigParameters);
    wifiManager.setAPCallback(handleConfigPortalStart);
    wifiManager.setTitle(PORTAL_TITLE);
}

void Wifi::saveConfigParameters() {
    if (!activeConfig || !activeConfigParameters) return;
    const char *ntpServerValue = activeConfigParameters->ntpServer().getValue();
    const char *timezoneValue = activeConfigParameters->timezoneHidden().getValue();
    const char *webLanguageValue = activeConfigParameters->webLanguage().getValue();
    const char *tempOffsetValue = activeConfigParameters->tempOffsetIndoor().getValue();
    const char *outdoorSensorChannelValue = activeConfigParameters->outdoorSensorChannel().getValue();
    const char *ventingThresholdValue = activeConfigParameters->ventingThreshold().getValue();
    const char *mqttHostValue = activeConfigParameters->mqttHost().getValue();
    const char *mqttPortValue = activeConfigParameters->mqttPort().getValue();
    const char *mqttUsernameValue = activeConfigParameters->mqttUsername().getValue();
    const char *mqttPasswordValue = activeConfigParameters->mqttPassword().getValue();

    if (WifiConfigFields::isValidNtpServer(ntpServerValue)) activeConfig->ntpServer = ntpServerValue;
    if (WifiConfigFields::isSupportedTimezone(timezoneValue)) activeConfig->timezonePosix = timezoneValue;
    if (WifiConfigFields::isSupportedWebLanguage(webLanguageValue)) {
        activeConfig->webLanguage = webLanguageValue;
    }
    if (WifiConfigFields::isValidMqttHost(mqttHostValue)) activeConfig->mqttHost = mqttHostValue;
    if (WifiConfigFields::isValidMqttCredential(mqttUsernameValue)) activeConfig->mqttUsername = mqttUsernameValue;
    if (WifiConfigFields::isValidMqttCredential(mqttPasswordValue)) activeConfig->mqttPassword = mqttPasswordValue;
    float parsedIndoorTemperatureOffset = 0.0f;
    if (WifiConfigFields::parseIndoorTemperatureOffset(tempOffsetValue, parsedIndoorTemperatureOffset)) {
        activeConfig->tempOffsetIndoor = parsedIndoorTemperatureOffset;
    }
    uint8_t parsedOutdoorSensorChannel = 0;
    if (WifiConfigFields::parseOutdoorSensorChannel(outdoorSensorChannelValue, parsedOutdoorSensorChannel)) {
        activeConfig->outdoorSensorChannel = parsedOutdoorSensorChannel;
    }
    uint16_t parsedMqttPort = 0;
    if (WifiConfigFields::parseMqttPort(mqttPortValue, parsedMqttPort)) {
        activeConfig->mqttPort = parsedMqttPort;
    }
    float parsedVentingThreshold = 0.0f;
    if (WifiConfigFields::parseVentingThreshold(ventingThresholdValue, parsedVentingThreshold)) {
        activeConfig->ventingThreshold = parsedVentingThreshold;
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
    DEBUG_MSG("%s\n", WiFi.softAPIP().toString().c_str());
    DEBUG_MSG("%s\n", wifiManagerInstance->getConfigPortalSSID().c_str());

    activeDisplayRef().showConfigPortalSsid(wifiManagerInstance->getConfigPortalSSID());
}

bool Wifi::connect(Display &screen, AppConfig &config) {
    setActiveDisplay(screen);
    activeConfig = &config;
    mdnsReady = false;

    prepareConfigPortalParameters(config);

    String hostname(HOSTNAME);
    WiFi.hostname(hostname);

    if (!wifiManager.autoConnect(HOSTNAME)) {
        DEBUG_MSG("WiFiManager failed to establish a connection.\n");
        return false;
    }

    DEBUG_MSG("Enabling WIFI...\n");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        DEBUG_MSG(".");
        activeDisplayRef().appendWifiConnectionProgress();
    }
    DEBUG_MSG("\n");

    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_MSG("WiFi did not reach WL_CONNECTED within the startup window.\n");

        return false;
    }

    mdnsReady = MDNS.begin(HOSTNAME);
    if (!mdnsReady) {
        DEBUG_MSG("mDNS responder failed to start.\n");
    }

    wasConnected = true;
    return true;
}
void Wifi::configureMqtt(const AppConfig &config) {
    mqttHost = config.mqttHost;
    mqttPort = config.mqttPort;
    mqttUsername = config.mqttUsername;
    mqttPassword = config.mqttPassword;
    mqttClient.disconnect();
    mqttClient.setServer(mqttHost.c_str(), mqttPort);
    mqttClient.setBufferSize(1024);
    lastMqttAttempt = 0;
}

bool Wifi::mqttEnabled() const {
    return mqttHost.length() != 0;
}

void Wifi::publishDiscovery() {
    for (size_t index = 0; index < MqttDiscovery::ENTITY_COUNT; ++index) {
        const std::string topic = MqttDiscovery::discoveryTopic(index);
        const std::string payload = MqttDiscovery::discoveryPayload(index, VERSION_STRING);
        mqttClient.publish(topic.c_str(), payload.c_str(), true);
    }
}

bool Wifi::connectMqtt() {
    if (!mqttEnabled() || WiFi.status() != WL_CONNECTED || mqttClient.connected()) return false;
    const unsigned long now = millis();
    if (now - lastMqttAttempt < 5000) return false;
    lastMqttAttempt = now;

    bool connected = false;
    if (mqttUsername.length() == 0 && mqttPassword.length() == 0) {
        connected = mqttClient.connect(HOSTNAME, MqttDiscovery::AVAILABILITY_TOPIC, 0, true,
                                       MqttDiscovery::PAYLOAD_NOT_AVAILABLE);
    } else {
        connected = mqttClient.connect(HOSTNAME, mqttUsername.c_str(), mqttPassword.c_str(),
                                       MqttDiscovery::AVAILABILITY_TOPIC, 0, true,
                                       MqttDiscovery::PAYLOAD_NOT_AVAILABLE);
    }
    if (!connected) return false;

    mqttClient.publish(MqttDiscovery::AVAILABILITY_TOPIC, MqttDiscovery::PAYLOAD_AVAILABLE, true);
    publishDiscovery();
    return true;
}

void Wifi::publishSensorStates(const DataJsonPayload::Payload &payload) {
    if (!mqttClient.connected()) return;

    const std::vector<MqttDiscovery::StateMessage> messages = MqttDiscovery::stateMessages(payload);
    for (const MqttDiscovery::StateMessage &message : messages) {
        mqttClient.publish(message.topic.c_str(), message.payload.c_str(), true);
    }
}

bool Wifi::isMdnsReady() const {
    return mdnsReady;
}

bool Wifi::isMqttConnected() const {
    return const_cast<PubSubClient &>(mqttClient).connected();
}

bool Wifi::poll() {
    doubleResetDetector.loop();

    // The ESP8266 core reconnects WiFi on its own, but mDNS stays silent
    // afterwards until re-announced.
    const bool connectedNow = (WiFi.status() == WL_CONNECTED);
    if (connectedNow && !wasConnected && mdnsReady) {
        DEBUG_MSG("WiFi reconnected, re-announcing mDNS.\n");
        MDNS.announce();
    }
    wasConnected = connectedNow;
    const bool mqttJustConnected = connectMqtt();
    if (mqttClient.connected()) mqttClient.loop();
    return mqttJustConnected;
}
