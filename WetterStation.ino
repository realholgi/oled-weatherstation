#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Wire.h>
#include <Adafruit_HTU21DF.h> // https://github.com/adafruit/Adafruit_HTU21DF_Library
#include <SensorReceiver.h>   // https://github.com/realholgi/433MHzForArduino/tree/master/RemoteSensor
#include <Adafruit_SSD1305.h> // https://github.com/adafruit/Adafruit_SSD1305_Library ???
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <DoubleResetDetector.h>  // https://github.com/datacute/DoubleResetDetector
#include <Ticker.h>
#include <TimeClient.h> // https://github.com/squix78/esp8266-weather-station

#include "Ubidots.h"
#include "icons.h"
#include "globals.h"
#include "PAGE_Wetter.h"

////// TRY: https://github.com/squix78/esp8266-oled-ssd1306  but cannot rotate
////// #include "SH1106.h"
////// SH1106Wire display(0x3D, SDA, SCL);

#include <FormattingSerialDebug.h> // https://github.com/rlogiacco/MicroDebug


Adafruit_SSD1305 display(OLED_RESET);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
ESP8266WebServer HTTP(80);
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
Ticker flasher;

volatile float humidity_outdoor = 0; // volatile var is stored in RAM (not register) for interrupvar_doesnt_exist handler - See more at: http://www.esp8266.com/viewtopic.php?f=28&t=9702&start=4#sthash.zNy41Ef8.dpuf
float humidity_indoor = 0;
volatile float temperature_outdoor = -273;
float temperature_indoor = -273;
float humidity_abs_indoor = -1;
float dp_indoor = -273;
volatile float humidity_abs_outdoor = -1;

bool shouldSaveConfig = false;
bool initialConfig = false;

volatile uint32_t upload_beginWait = millis() + UBIDOTS_MIN_UPLOAD_INTERVAL;
volatile uint32_t last_received_ext = millis() + MIN_RECEIVE_WAIT_EXT + 1;
volatile uint32_t last_received_int = millis() + MIN_RECEIVE_WAIT_INT + 1;
String prevDisplay = "--"; // when the digital clock was displayed

const char* configPortalPassword = PORTAL_DEFAULT_PASSWORD;

//  additional portal parameters
char UBIDOTS_API_KEY[40] = "";
uint32_t TIMEZONE = 1;

TimeClient timeClient(TIMEZONE);

#include "display.h"

void setup()   {
  SERIAL_DEBUG_SETUP(115200);

  DEBUG("compiled: %s %s\n", __DATE__, __TIME__);

  DEBUG("FW %s\n", FIRMWAREVERSION);
  DEBUG("SDK: %s\n", ESP.getSdkVersion());

  ESP.wdtDisable();
  ESP.wdtEnable(2000);  // Enable it again with a longer wait time ( 2 seconds instead of the default 1 second )

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  display.begin();
  display.setRotation(3);
  display.clearDisplay();
  display.display();

  display.setTextWrap(false);
  display.setTextColor(WHITE);
  display.setTextSize(1);

  if (!htu.begin()) {
    printAt(6, 0, "ERROR:", false);
    printAt(6, 20, "internal", false);
    printAt(6, 30, "Sensor", false);
    printAt(6, 30, "failed!");
    DEBUG("Couldn't find local sensor!\n");
    while (1);
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

  timeClient = TimeClient(TIMEZONE);
  timeClient.updateTime();

  printAt(6, 40, "433MHz...");
  SensorReceiver::init(RECEIVER_PIN, getRemoteTempHumi);

  printAt(6, 50, "OTA...");
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onProgress(drawOtaProgress);
  ArduinoOTA.onEnd(drawOtaEnd);
  //ArduinoOTA.setPassword(PORTAL_DEFAULT_PASSWORD);
  ArduinoOTA.begin();

  DEBUG("Ready\n");
}

void loop() {
  drd.loop();
  HTTP.handleClient();
  ArduinoOTA.handle();
  MDNS.update();

  updateInternalSensor();

  displayData();

  uploadData();

  delay(1000);
}

void updateInternalSensor() {
  if (millis() - last_received_int < MIN_RECEIVE_WAIT_INT) {
    return;
  }

  humidity_indoor = htu.readHumidity();
  temperature_indoor = htu.readTemperature();

  if (temperature_indoor > -273 && humidity_indoor > 0) {
    humidity_abs_indoor = berechneTT(temperature_indoor, humidity_indoor);
    dp_indoor = RHtoDP(temperature_indoor, humidity_indoor);
    last_received_int = millis();
  }
}

boolean shouldStartSetup() {
  if ( (!loadConfig()) || WiFi.SSID() == "") {
    DEBUG("No stored access-point credentials; initiating configuration portal.");
    display.clearDisplay();
    printAt(6, 0, "no Cfg", false);
    printAt(6, 10, "Portal");
    delay(1000);
    initialConfig = true;
    return true;
  }

  if (drd.detectDoubleReset()) {
    DEBUG("Double-reset detected...");
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

  WiFiManagerParameter custom_UBIDOTS_API_KEY("UDkey", "UBIDOTS API key", UBIDOTS_API_KEY, 40, "<p>UBIDOTS API Key</p");
  WiFiManagerParameter custom_TIMEZONE("TIMEZONE", "Timezone",  String(TIMEZONE).c_str(), 5, TYPE_NUMBER);

  if (initialConfig) {
    DEBUG("Starting configuration portal.");

    flasher.attach(0.1, flash);

    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPCallback(configModeCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_UBIDOTS_API_KEY);
    wifiManager.addParameter(&custom_TIMEZONE);

    //it starts an access point
    //and goes into a blocking loop awaiting configuration

    String hostname;
    hostname = "ESP" + String(ESP.getChipId(), HEX);
    DEBUG("Hostname:  "); Serial.println(hostname);

    if (!wifiManager.startConfigPortal(HOSTNAME)) {
      DEBUG("Not connected to WiFi but continuing anyway.");
    } else {
      //if you get here you have connected to the WiFi
      DEBUG("Connected to WiFi.");

      //read updated parameters
      strcpy(UBIDOTS_API_KEY, custom_UBIDOTS_API_KEY.getValue());
      TIMEZONE = String(custom_TIMEZONE.getValue()).toInt();

      //save the custom parameters to FS
      if (shouldSaveConfig) {
        DEBUG("saving config");
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.createObject();
        json["UBIDOTS_API_KEY"] = UBIDOTS_API_KEY;
        json["TIMEZONE"] = TIMEZONE;

        File configFile = SPIFFS.open(CONFIG_FILE, "w");
        if (!configFile) {
          DEBUG("failed to open config file for writing");
        }

        json.printTo(Serial);
        json.printTo(configFile);
        DEBUG("fnished saving configuration file, closing");
        configFile.close();
        //end save
      }
    }
    ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up
    // so resetting the device allows to go back into config mode again when it reboots.
    delay(5000);
  }
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
}


//callback notifying us of the need to save config
void saveConfigCallback () {
  DEBUG("Should save config");
  shouldSaveConfig = true;
}

void flash() {
  // blink the LED
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

boolean loadConfig() {
  //read configuration from FS json
  DEBUG("mounting FS...");

  if (SPIFFS.begin()) {
    DEBUG("mounted file system");

    // Uncomment following lines to delete the CONFIG_FILE file...
    /*if (SPIFFS.remove(CONFIG_FILE)) {
      DEBUG("***Successfully deleted CONFIG_FILE file...***");
      } else {
      DEBUG("***...error deleting CONFIG_FILE file...***");
      }
    */

    if (SPIFFS.exists(CONFIG_FILE)) {
      //file exists, reading and loading
      DEBUG("reading config file");
      File configFile = SPIFFS.open(CONFIG_FILE, "r");
      if (configFile) {
        DEBUG("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          DEBUG("\nparsed json");
          strcpy(UBIDOTS_API_KEY, json["UBIDOTS_API_KEY"]);
          TIMEZONE = json["TIMEZONE"];
          DEBUG("...finished...");
          return true;
        } else {
          DEBUG("failed to load json config");
        }
      }
    } else {
      DEBUG("CONFIG_FILE does not exist.");
    }
  } else {
    DEBUG("failed to mount FS");
  }
  //end read
  return false;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG("Entered config mode");
  DEBUG(WiFi.softAPIP().toString().c_str());
  DEBUG(myWiFiManager->getConfigPortalSSID().c_str());

  display.clearDisplay();
  display.setTextSize(1);
  printAt(6, 0, "SETUP", false);
  printAt(6, 20, "SSID:", false);
  printAt(6, 40, myWiFiManager->getConfigPortalSSID());
}

void setupWIFI() {
  WiFiManager wifiManager; //Local intialization. Once its business is done, there is no need to keep it around

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

  DEBUG("Enabling WIFI...\n");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    DEBUG(".");
    display.print(".");
    display.display();
  }
  DEBUG("\n");
  MDNS.begin("wetter"); //find it as http://wetter.local
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

void uploadData() {
  if (millis() - upload_beginWait < UBIDOTS_MIN_UPLOAD_INTERVAL) {
    return;
  }

  Ubidots ubiclient(UBIDOTS_API_KEY, UBI_HOSTNAME);
  ubiclient.setDataSourceName(UBI_HOSTNAME);

  DEBUG("Uploading to Ubidots...\n");

  if (humidity_indoor > 0)
    ubiclient.add("h_in", int(humidity_indoor));

  if (temperature_indoor > -273)
    ubiclient.add("t_in", temperature_indoor);

  if (humidity_outdoor > 0)
    ubiclient.add("h_out", int(humidity_outdoor));

  if ( temperature_outdoor > -273)
    ubiclient.add("t_out", temperature_outdoor);

  ubiclient.sendAll(false);

  if (humidity_indoor > 0 && temperature_indoor  > -273) {
    ubiclient.add("f_in", humidity_abs_indoor);
    ubiclient.add("dp_in", dp_indoor);
  }

  if (humidity_outdoor > 0 && temperature_outdoor > -273)
    ubiclient.add("f_out", humidity_abs_outdoor);

  if (temperature_outdoor > -273 && humidity_outdoor > 0 && temperature_indoor > -273 && humidity_indoor > 0)
    ubiclient.add("f_diff", humidity_abs_indoor - humidity_abs_outdoor);

  ubiclient.sendAll(false);

  upload_beginWait = millis();
}

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

  display.drawBitmap(2, 20 + OFFSET,  sun_icon16x16, 16, 16, WHITE);

  // ---------------------------------
  display.drawLine(0, 40 + OFFSET, display.width() - 1, 40 + OFFSET, WHITE);

  display.setTextSize(2);
  if (temperature_indoor > -273) {
    printNumF(6, 40 + 4, temperature_indoor);
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

  display.drawBitmap(2, 20 + 40 + 2 + OFFSET,  home_icon16x16, 16, 16, WHITE);

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
    display.drawBitmap(0, 3 + 82 + OFFSET,  warning_icon16x16, 16, 16, color);
  }

  if (timeClient.getHours() != prevDisplay) { // update internet time every hour
    timeClient.updateTime();
    prevDisplay = timeClient.getHours();
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
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  json["t_in"] = temperature_indoor;
  json["h_in"] = int(humidity_indoor);
  json["f_in"] = humidity_abs_indoor;
  json["dp_in"] = dp_indoor;

  json["t_out"] = temperature_outdoor;
  json["h_out"] = int(humidity_outdoor);
  json["f_out"] = humidity_abs_outdoor;

  json["f_diff"] = humidity_abs_indoor - humidity_abs_outdoor;

  String message = "";
  json.printTo(message);
  HTTP.send(200, "application/json;charset=utf-8", message);
}

void handleRoot() {
  HTTP.send(200, "text/html", PAGE_Wetter);
}

ICACHE_RAM_ATTR void getRemoteTempHumi(byte * data) {

  if (millis() - last_received_ext < MIN_RECEIVE_WAIT_EXT) {
    return;
  }

  if ((data[3] & 0x1f) == THERMO_HYGRO_DEVICE) {   // is data a ThermoHygro-device?

    byte channel, randomId;
    int temp;
    byte humidity;

    SensorReceiver::decodeThermoHygro(data, channel, randomId, temp, humidity);

    if (randomId == MY_RF_RECEIVER_ID) {
      humidity_outdoor = humidity;
      temperature_outdoor = temp / 10.0;

      if (temperature_outdoor > -273 && humidity_outdoor > 0) {
        humidity_abs_outdoor = berechneTT(temperature_outdoor, humidity_outdoor);
      }

      last_received_ext = millis();
    } else {
      DEBUG("ROGUE SENSOR FOUND!");
    }
    DEBUG("Temperature: %u.%u deg, Humidity: %u % REL, ID: %u\n", temp / 10, abs(temp % 10), humidity, randomId);
  }
}

// absolute Feuchtegehalt der Luft in Gramm Wasserdampf pro Kubikmeter
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
// dv = 216.7*(RH/100.0*6.112*exp(17.62*t/(243.12+t))/(273.15+t));
ICACHE_RAM_ATTR double berechneTT(double t, double RH)
{
  double absFeuchte = 216.7f * (RH / 100.0f * 6.112f * exp(WATER_VAPOR * t / (BAROMETRIC_PRESSURE + t)) / (273.15f + t));
  return  absFeuchte;
}

// Berechne Taupunkt
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/2_Humidity_Sensors/Sensirion_Humidity_Sensors_at_a_Glance_V1.pdf
double RHtoDP(double t, double RH) {
  double H = (log10(RH) - 2.0) / 0.4343 + (WATER_VAPOR * t) / (BAROMETRIC_PRESSURE + t);
  double td = BAROMETRIC_PRESSURE * H / (WATER_VAPOR - H);
  return td;
}

