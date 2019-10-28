//
// Holgi's Wetterstation V1.1
//
//
// Platform: ESP8266 Wemos D1 Mini 1M SPIFFS
//
//

#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_HTU21DF.h> // https://github.com/adafruit/Adafruit_HTU21DF_Library
#include <fws433.h>   // https://github.com/realholgi/FWS433
#include <Adafruit_SSD1305.h> // https://github.com/adafruit/Adafruit_SSD1305_Library ???
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <DoubleResetDetector.h>  // https://github.com/datacute/DoubleResetDetector
#include <Ticker.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

#include "proto.h"

#include "TimeClient.h"
//#include "Ubidots.h"
#include "icons.h"
#include "globals.h"
#include "PAGE_Wetter.h"

////// TRY: https://github.com/squix78/esp8266-oled-ssd1306  but cannot rotate
////// #include "SH1106.h"
////// SH1106Wire display(0x3D, SDA, SCL);

#define XUBIDOTS


Adafruit_SSD1305 display(OLED_RESET);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
ESP8266WebServer HTTP(80);
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

Ticker tickerForInternalSensorUpdate;
Ticker tickerForUploadData;
Ticker tickerForTimeUpdate;
Ticker tickerForExternalSensorInvalidate;

volatile float humidity_outdoor = 0; // volatile var is stored in RAM (not register) for interrupvar_doesnt_exist handler - See more at: http://www.esp8266.com/viewtopic.php?f=28&t=9702&start=4#sthash.zNy41Ef8.dpuf
float humidity_indoor = 0;
volatile float temperature_outdoor = -273;
volatile int battery_outdoor = 0;
float temperature_indoor = -273;
float humidity_abs_indoor = -1;
float dp_indoor = -273;
volatile float humidity_abs_outdoor = -1;

WiFiManager wifiManager;

FWS433 fws = FWS433();

bool shouldSaveConfig = false;
bool initialConfig = false;

volatile uint32_t last_received_ext = millis() + MIN_RECEIVE_WAIT_EXT + 1;

bool readyForInternalSensorUpdate = true;
bool readyForUploadData = false;
bool readyForTimeUpdate = false;

const char *configPortalPassword = PORTAL_DEFAULT_PASSWORD;

uint32_t TIMEZONE = 1;

TimeClient timeClient(TIMEZONE);

#include "display.h"

void setup() {
    Serial.begin(115200);

    DEBUG_MSG("compiled: %s %s\n", __DATE__, __TIME__);

    DEBUG_MSG("FW %s\n", FIRMWAREVERSION);
    DEBUG_MSG("SDK: %s\n", ESP.getSdkVersion());

    //ESP.wdtDisable();
    //ESP.wdtEnable(2000);  // Enable it again with a longer wait time ( 2 seconds instead of the default 1 second )

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    display.begin();
    display.setRotation(3);
    display.clearDisplay();
    display.display();

    display.setTextWrap(false);
    display.setTextColor(WHITE);
    display.setTextSize(1);

    if (!htu.begin()) {
        printAt(6, 0, "ERROR:", false);
        printAt(6, 20, "local", false);
        printAt(6, 30, "Sensor", false);
        printAt(6, 40, "failing!");
        DEBUG_MSG("Couldn't find local sensor!\n");
        while (1) {
            yield();
        }
    }

    printAt(6, 0, "Config...");
    if (shouldStartSetup()) {
        doSetup();
    }

    printAt(6, 10, "WIFI");
    setupWIFI();

    printAt(6, 20, "HTTP...");
    setupWebserver();

    printAt(6, 30, "Time...");

    timeClient = TimeClient(TIMEZONE); // set TZ value from config
    timeClient.updateTime();

    printAt(6, 40, "433MHz...");
    fws.start(RECEIVER_PIN);

    printAt(6, 50, "OTA...");
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.onProgress(drawOtaProgress);
    ArduinoOTA.onEnd(drawOtaEnd);
    //ArduinoOTA.setPassword(PORTAL_DEFAULT_PASSWORD);
    ArduinoOTA.begin();

    tickerForInternalSensorUpdate.attach(MIN_RECEIVE_WAIT_INT, setReadyForInternalSensorUpdate);
#ifdef UBIDOTS
    tickerForUploadData.attach(UBIDOTS_UPLOAD_INTERVAL, setReadyForUploadData);
#endif
    tickerForTimeUpdate.attach(UPDATE_NTP_TIME_INTERVAL, setReadyForTimeUpdate);
    tickerForExternalSensorInvalidate.attach(MAX_RECEIVE_WAIT_EXT / 2, setExternalSensorInvalid);

    DEBUG_MSG("Ready\n");
}

void loop() {
    drd.loop();
    HTTP.handleClient();
    ArduinoOTA.handle();

    if (readyForInternalSensorUpdate) {
        updateInternalSensor();
    }

    if (readyForTimeUpdate) {
        updateTime();
    }

    if (fws.isDataAvailable()) {
        updateExternalSensor();
    }

    displayData();

#ifdef UBIDOTS
    if (readyForUploadData) {
      uploadData();
    }
#endif

    MDNS.update();

    delay(1000);
    yield();
}

void updateExternalSensor() {
    fwsResult result = fws.getData();

    if (result.channel == 3) {
        last_received_ext = millis();

        humidity_outdoor = result.humidity;
        temperature_outdoor = result.temperature / 10.0;

        if (temperature_outdoor > -40 && temperature_outdoor < 50 && humidity_outdoor > 0 && humidity_outdoor < 100) {
            humidity_abs_outdoor = berechneTT(temperature_outdoor, humidity_outdoor);
        }

        DEBUG_MSG("Temperature: %u.%u deg, Humidity: %u % REL, ID: %u\n", result.temperature / 10,
                  abs(result.temperature % 10), result.humidity,
                  result.id);

    }
};

void updateTime() {
    timeClient.updateTime();
    readyForTimeUpdate = false;
}

void updateInternalSensor() {
    humidity_indoor = htu.readHumidity();
    temperature_indoor = htu.readTemperature();

    if (temperature_indoor > -273 && humidity_indoor > 0) {
        humidity_abs_indoor = berechneTT(temperature_indoor, humidity_indoor);
        dp_indoor = RHtoDP(temperature_indoor, humidity_indoor);
        readyForInternalSensorUpdate = false;
    }
}

void setExternalSensorInvalid() {
    if (millis() - last_received_ext > MAX_RECEIVE_WAIT_EXT) {
        DEBUG_MSG("No External Sensor Signal received for a long time!");
        temperature_outdoor = -273;
        humidity_outdoor = 0;
    }
}

bool shouldStartSetup() {
    if (WiFi.SSID() == "") {
        DEBUG_MSG("No stored access-point credentials; initiating configuration portal.");
        display.clearDisplay();
        printAt(6, 0, "no Cfg", false);
        printAt(6, 10, "Portal");
        delay(1000);
        initialConfig = true;
        return true;
    }

    if (drd.detectDoubleReset()) {
        DEBUG_MSG("Double-reset detected...");
        display.clearDisplay();
        printAt(6, 0, "RESET", false);
        printAt(6, 10, "Portal");
        delay(1000);
        initialConfig = true;
        return true;
    }
    return false;
}

void doSetup() {

    if (initialConfig) {
        DEBUG_MSG("Starting configuration portal.");

        Ticker flasher;
        flasher.attach(0.1, flash);

        wifiManager.setSaveConfigCallback(saveConfigCallback);
        wifiManager.setAPCallback(configModeCallback);

        String hostname;
        hostname = "ESP" + String(ESP.getChipId(), HEX);
        DEBUG_MSG("Hostname:  ");
        Serial.println(hostname);

        if (!wifiManager.startConfigPortal(HOSTNAME)) {
            DEBUG_MSG("Not connected to WiFi but continuing anyway.");
        } else {
            //if you get here you have connected to the WiFi
            DEBUG_MSG("Connected to WiFi.");
        }
        ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up
        // so resetting the device allows to go back into config mode again when it reboots.
        delay(5000);
    }
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.mode(
            WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
}


//callback notifying us of the need to save config
void saveConfigCallback() {
    DEBUG_MSG("Should save config");
    shouldSaveConfig = true;
}

void flash() {
    // blink the LED
    int state = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !state);
}

void configModeCallback(WiFiManager *myWiFiManager) {
    DEBUG_MSG("Entered config mode");
    DEBUG_MSG(WiFi.softAPIP().toString().c_str());
    DEBUG_MSG(myWiFiManager->getConfigPortalSSID().c_str());

    display.clearDisplay();
    display.setTextSize(1);
    printAt(6, 0, "SETUP", false);
    printAt(6, 20, "SSID:", false);
    printAt(6, 40, myWiFiManager->getConfigPortalSSID());
}

void setupWIFI() {
    
    // Uncomment for testing wifi manager
    // wifiManager.resetSettings();
    wifiManager.setAPCallback(configModeCallback);

    //or use this for auto generated name ESP + ChipID
    wifiManager.autoConnect();

    //Manual Wifi
    // WiFi.begin(SSID, PASSWORD);
    String hostname(HOSTNAME);
    //hostname += String(ESP.getChipId(), HEX);
    WiFi.hostname(hostname);

    DEBUG_MSG("Enabling WIFI...\n");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        DEBUG_MSG(".");
        display.print(".");
        display.display();
    }
    DEBUG_MSG("\n");
    MDNS.begin(HOSTNAME);
}

void drawOtaProgress(unsigned int progress, unsigned int total) {
    display.clearDisplay();
    display.setTextSize(1);
    printAt(6, 0, "OTA...", false);
    String percent = (progress / (total / 100)) + "%";
    printAt(6, 10, percent);
}

void drawOtaEnd() {
    printAt(6, 20, "OK, Reboot");
}

#ifdef UBIDOTS
void uploadData() {
    Ubidots ubiclient(UBIDOTS_API_KEY, UBI_HOSTNAME);
    ubiclient.setDataSourceName(UBI_HOSTNAME);

    DEBUG_MSG("Uploading to Ubidots...\n");

    if (humidity_indoor > 0)
        ubiclient.add("h_in", int(humidity_indoor));

    if (temperature_indoor > -273)
        ubiclient.add("t_in", temperature_indoor);

    if (humidity_outdoor > 0)
        ubiclient.add("h_out", int(humidity_outdoor));

    if (temperature_outdoor > -273)
        ubiclient.add("t_out", temperature_outdoor);

    ubiclient.sendAll(false);

    if (humidity_indoor > 0 && temperature_indoor > -273) {
        ubiclient.add("f_in", humidity_abs_indoor);
        ubiclient.add("dp_in", dp_indoor);
    }

    if (humidity_outdoor > 0 && temperature_outdoor > -273)
        ubiclient.add("f_out", humidity_abs_outdoor);

    if (temperature_outdoor > -273 && humidity_outdoor > 0 && temperature_indoor > -273 && humidity_indoor > 0)
        ubiclient.add("f_diff", humidity_abs_indoor - humidity_abs_outdoor);

    ubiclient.sendAll(false);

    readyForUploadData = false;
}
#endif

void displayData() {
    display.clearDisplay();
    display.setTextSize(2);

    if (temperature_outdoor > -273) {
        printNumF(6, 0, temperature_outdoor);
    }

    display.setTextSize(1);
    if (humidity_outdoor > 0) {
        printNumI(46, 20, humidity_outdoor);
        display.print("%");
    }

    display.setTextSize(1);
    if (humidity_abs_outdoor > -1) {
        printNumF(34, 30, humidity_abs_outdoor);
    }

    display.drawBitmap(2, 20 + OFFSET, sun_icon16x16, 16, 16, WHITE);

    // ---------------------------------
    display.drawLine(0, 40 + OFFSET, display.width() - 1, 40 + OFFSET, WHITE);

    display.setTextSize(2);
    if (temperature_indoor > -273) {
        printNumF(6, 40 + 4, temperature_indoor - TEMP_OFFSET_INDOOR);
    }

    display.setTextSize(1);
    if (humidity_indoor > 0) {
        printNumI(46, 20 + 40 + 2, humidity_indoor);
        display.print("%");
    }

    display.setTextSize(1);
    if (humidity_abs_indoor > -1) {
        printNumF(34, 30 + 40 + 2, humidity_abs_indoor);
    }

    display.drawBitmap(2, 20 + 40 + 2 + OFFSET, home_icon16x16, 16, 16, WHITE);

    display.drawLine(0, 82 + OFFSET, display.width() - 1, 82 + OFFSET, WHITE);

    // -----------

    display.setTextSize(2);

    if (temperature_outdoor > -273 && humidity_outdoor > 0 && temperature_indoor > -273 && humidity_indoor > 0) {
        float diff = humidity_abs_indoor - humidity_abs_outdoor;
        printNumF(6, 82 + 4, diff);
        int color = WHITE;
        if (diff > 0.0 && abs(diff) < MIN_DIFF) {
            color = BLACK;
        }
        display.drawBitmap(0, 3 + 82 + OFFSET, warning_icon16x16, 16, 16, color);
    }

    String time = timeClient.getFormattedTime().substring(0, 5);
    printAt(6, 82 + 4 + 22, time);
}

void setupWebserver() {
    HTTP.on("/", handleRoot);
    HTTP.on("/data.json", HTTP_GET, [&]() {
        HTTP.sendHeader("Connection", "close");
        HTTP.sendHeader("Access-Control-Allow-Origin", "*");
        return handleJsonData();
    });
    HTTP.onNotFound(handleNotFound);

    HTTP.begin();

    MDNS.addService("http", "tcp", 80);
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += HTTP.uri();
    message += "\nMethod: ";
    message += (HTTP.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += HTTP.args();
    message += "\n";
    for (uint8_t i = 0; i < HTTP.args(); i++) {
        message += " " + HTTP.argName(i) + ": " + HTTP.arg(i) + "\n";
    }
    HTTP.send(404, "text/plain", message);
}

void handleJsonData() {
    DynamicJsonDocument doc(1024);

    doc["t_in"] = temperature_indoor - TEMP_OFFSET_INDOOR;
    doc["h_in"] = int(humidity_indoor);
    doc["f_in"] = humidity_abs_indoor;
    doc["dp_in"] = dp_indoor;

    doc["t_out"] = temperature_outdoor;
    doc["h_out"] = int(humidity_outdoor);
    doc["f_out"] = humidity_abs_outdoor;
    doc["b_out"] = battery_outdoor;
    doc["last_out"] = int(millis() - last_received_ext) / 1000;

    doc["f_diff"] = humidity_abs_indoor - humidity_abs_outdoor;

    String message = "";
    serializeJson(doc, message);

    HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
    HTTP.send_P(200, "text/html", PAGE_Wetter);
}

void setReadyForInternalSensorUpdate() {
    readyForInternalSensorUpdate = true;
}

void setReadyForTimeUpdate() {
    readyForTimeUpdate = true;
}

void setReadyForUploadData() {
    readyForUploadData = true;
}

// absolute Feuchtegehalt der Luft in Gramm Wasserdampf pro Kubikmeter
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
// dv = 216.7*(RH/100.0*6.112*exp(17.62*t/(243.12+t))/(273.15+t));
ICACHE_RAM_ATTR double berechneTT(double t, double RH) {
    double absFeuchte =
            216.7f * (RH / 100.0f * 6.112f * exp(WATER_VAPOR * t / (BAROMETRIC_PRESSURE + t)) / (273.15f + t));
    return absFeuchte;
}

// Berechne Taupunkt
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
double RHtoDP(double t, double RH) {
    double H = (log10(RH) - 2.0) / 0.4343 + (WATER_VAPOR * t) / (BAROMETRIC_PRESSURE + t);
    double td = BAROMETRIC_PRESSURE * H / (WATER_VAPOR - H);
    return td;
}

