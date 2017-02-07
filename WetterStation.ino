#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <Wire.h>
#include <Adafruit_HTU21DF.h> // https://github.com/adafruit/Adafruit_HTU21DF_Library
#include <SensorReceiver.h>   // https://github.com/realholgi/433MHzForArduino/tree/master/RemoteSensor
#include <Adafruit_SSD1305.h> // https://github.com/adafruit/Adafruit_SSD1305_Library ???
#include <Adafruit_GFX.h>
#include "Ubidots.h"
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

#include "icons.h"
#include "globals.h"

////// TRY: https://github.com/squix78/esp8266-oled-ssd1306  but cannot rotate
////// #include "SH1106.h"
////// SH1106Wire display(0x3D, SDA, SCL);

#define SERIAL_DEBUG true
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
volatile uint32_t receive_beginWait = millis() + MIN_RECEIVE_INTERVAL + 1;
volatile uint32_t tempread_beginWait = millis() + MIN_TEMP_READ_INTERVAL + 1;
String prevDisplay = "--"; // when the digital clock was displayed

const char* configPortalPassword = PORTAL_DEFAULT_PASSWORD;

char UBIDOTS_API_KEY[40] = "";
uint32_t TIMEZONE = 1;

TimeClient timeClient(TIMEZONE);

void printNumI(int num);
void printNumF (double num, byte dec = 1, int length = 0);

void setup()   {
  SERIAL_DEBUG_SETUP(115200);

  DEBUG("compiled: %s %s\n", __DATE__, __TIME__);

  DEBUG("FW %s\n", FIRMWAREVERSION);
  DEBUG("SDK: %s\n", ESP.getSdkVersion());

  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  if (!htu.begin()) {
    DEBUG("Couldn't find local sensor!\n");
    while (1);
  }

  // OLED: Turn On VCC
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);

  display.begin();
  display.setRotation(3);
  display.clearDisplay();
  display.display();

  display.setTextWrap(false);
  display.setTextColor(WHITE);

  display.setTextSize(1);

  display.setCursor(6, 0 + OFFSET);
  display.print(F("Config..."));
  display.display();

  doSetup();

  display.setCursor(6, 10 + OFFSET);
  display.print(F("WIFI..."));
  display.display();

  setupWIFI2();

  display.setCursor(6, 20  + OFFSET);
  display.print(F("HTTP..."));
  display.display();

  setupWebserver();

  display.setCursor(6, 30  + OFFSET);
  display.print(F("Time..."));
  display.display();

  timeClient = TimeClient(TIMEZONE);
  timeClient.updateTime();

  SensorReceiver::init(RECEIVER_PIN, getRemoteTempHumi);

  ESP.wdtDisable();
  ESP.wdtEnable(2000);  // Enable it again with a longer wait time ( 2 seconds instead of the default 1 second )

  // Setup OTA
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.onProgress(drawOtaProgress);
  ArduinoOTA.onEnd(drawOtaEnd);
  //ArduinoOTA.setPassword(PORTAL_DEFAULT_PASSWORD);
  ArduinoOTA.begin();

  DEBUG("Ready\n");
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  DEBUG("Should save config");
  shouldSaveConfig = true;
}

void flash() {
  // triggers the LED
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void doSetup() {

  loadConfig2();

  if (WiFi.SSID() == "") {
    DEBUG("No stored access-point credentials; initiating configuration portal.");
    display.clearDisplay();
    display.setCursor(6, 0  + OFFSET);
    display.print(F("no Cfg"));
    display.setCursor(6, 10  + OFFSET);
    display.print(F("Portal"));
    display.display();
    delay(1000);
    initialConfig = true;
  }
  if (drd.detectDoubleReset()) {
    DEBUG("Double-reset detected...");
    display.clearDisplay();
    display.setCursor(6, 0  + OFFSET);
    display.print(F("RESET"));
    display.setCursor(6, 20  + OFFSET);
    display.print(F("Portal"));
    display.display();
    delay(1000);
    initialConfig = true;
  }

  WiFiManagerParameter custom_UBIDOTS_API_KEY("UDkey", "UBIDOTS API key", UBIDOTS_API_KEY, 40, "<p>UBIDOTS API Key</p");
  WiFiManagerParameter custom_TIMEZONE("TIMEZONE", "Timezone",  String(TIMEZONE).c_str(), 5, TYPE_NUMBER);

  if (initialConfig) {
    DEBUG("Starting configuration portal.");
    flasher.attach(0.1, flash);

    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // Uncomment for testing wifi manager
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
        Serial.println("saving config");
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

void loadConfig2() {
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
        } else {
          DEBUG("failed to load json config");
        }
      }
    } else {
      DEBUG("CONFIG_FILE does not exist.");
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
}

void configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG("Entered config mode");
  DEBUG(WiFi.softAPIP().toString().c_str());
  //if you used auto generated SSID, print it
  DEBUG(myWiFiManager->getConfigPortalSSID().c_str());
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(6, 0 + OFFSET);
  display.print(F("SETUP"));
  display.setCursor(6, 20 + OFFSET);
  display.print(F("SSID:"));
  display.setCursor(6, 30 + OFFSET);
  display.print(myWiFiManager->getConfigPortalSSID());
  display.display();
}

void setupWIFI2() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // Uncomment for testing wifi manager
  // wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  // WiFi.begin(SSID, PASSWORD);
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  DEBUG("Enabling WIFI...\n");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    DEBUG(".");
  }
  DEBUG("\n");
}

void drawOtaProgress(unsigned int progress, unsigned int total) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(6, 0 + OFFSET);
  display.print(F("OTA..."));
  display.setCursor(6, 10 + OFFSET);
  display.print(progress / (total / 100));
  display.print(F("%"));
  display.display();
}

void drawOtaEnd() {
  display.setCursor(6, 20 + OFFSET);
  display.print(F("OK, Reboot"));
  display.display();
}

void loop() {
  drd.loop();
  HTTP.handleClient();
  ArduinoOTA.handle();
  MDNS.update();

  if (millis() - tempread_beginWait > MIN_TEMP_READ_INTERVAL) {
    humidity_indoor = htu.readHumidity();
    temperature_indoor = htu.readTemperature();
    tempread_beginWait = millis();
    if (temperature_indoor > -273 && humidity_indoor > 0) {
      humidity_abs_indoor = berechneTT(temperature_indoor, humidity_indoor);
      dp_indoor = RHtoDP(temperature_indoor, humidity_indoor);
    }
  }

  displayData();

  uploadData();

  delay(1000);
}


void uploadData() {
  if (millis() - upload_beginWait < UBIDOTS_MIN_UPLOAD_INTERVAL) {
    return;
  }

  Ubidots ubiclient(UBIDOTS_API_KEY, HOSTNAME);
  ubiclient.setDataSourceName(HOSTNAME);

  DEBUG("Uploading to ubidots...\n");

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
  display.setCursor(6, 0 + OFFSET);
  if (temperature_outdoor > -273) {
    printNumF(temperature_outdoor);
  }

  display.setTextSize(1);
  display.setCursor(46, 20 + OFFSET);
  if (humidity_outdoor > 0) {
    printNumI(humidity_outdoor);
    display.print("%");
  }

  display.setTextSize(1);
  display.setCursor(34, 30 + OFFSET); //
  if (humidity_abs_outdoor > -1) {
    printNumF(humidity_abs_outdoor);
  }

  display.drawBitmap(2, 20 + OFFSET,  sun_icon16x16, 16, 16, WHITE);

  // ---------------------------------
  display.drawLine(0, 40 + OFFSET, display.width() - 1, 40 + OFFSET, WHITE);

  display.setTextSize(2);
  display.setCursor(6, 40 + 4 + OFFSET);
  if (temperature_indoor > -273) {
    printNumF(temperature_indoor);
  }

  display.setTextSize(1);
  display.setCursor(46, 20 + 40 + 2 + OFFSET);
  if (humidity_indoor > 0) {
    printNumI(humidity_indoor);
    display.print("%");
  }

  display.setTextSize(1);
  display.setCursor(34, 30 + 40 + 2 + OFFSET);
  if (humidity_abs_indoor > -1) {
    printNumF(humidity_abs_indoor);
  }

  display.drawBitmap(2, 20 + 40 + 2 + OFFSET,  home_icon16x16, 16, 16, WHITE);

  display.drawLine(0, 82 + OFFSET, display.width() - 1, 82 + OFFSET, WHITE);

  // -----------

  display.setTextSize(2);
  display.setCursor(6, 82 + 4 + OFFSET);
  if (temperature_outdoor > -273 && humidity_outdoor > 0 && temperature_indoor > -273 && humidity_indoor > 0) {
    float diff = humidity_abs_indoor - humidity_abs_outdoor;
    printNumF(diff);
    int color = WHITE;
    if (diff > 0.0 && abs(diff) < MIN_DIFF) {
      color = BLACK;
    }
    display.drawBitmap(0, 3 + 82 + OFFSET,  warning_icon16x16, 16, 16, color);
  }

  display.setCursor(6, 82 + 4 + 22 + OFFSET);

  if (timeClient.getHours() != prevDisplay) { // update internet time every hour
    timeClient.updateTime();
    prevDisplay = timeClient.getHours();
  }

  String time = timeClient.getFormattedTime().substring(0, 5);
  display.print(time);

  display.display();
}

void setupWebserver() {
  HTTP.on("/", handleRoot);
  HTTP.on("/data.json", HTTP_GET, [&]() {
    HTTP.sendHeader("Connection", "close");
    HTTP.sendHeader("Access-Control-Allow-Origin", "*");
    return handleData();
  });
  HTTP.onNotFound(handleNotFound);

  HTTP.begin();

  MDNS.begin("wetter"); //find it as http://wetter.local
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

void handleData() {
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
  String message = F("<!DOCTYPE html>\n");
  message += F("<html lang=\"de\">\n");
  message += F("<head>\n");
  message += F("  <title>Wetterstation</title>\n");
  message += F("  <meta charset=\"utf-8\">\n");
  message += F("  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n");
  message += F("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
  message += F("<!-- The above 3 meta tags *must* come first in the head; any other head content must come *after* these tags -->\n");
  message += F("  <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">\n");
  message += F("  <link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css\" integrity=\"sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp\" crossorigin=\"anonymous\">\n");
  message += F("<!--[if lt IE 9]>\n");
  message += F("  <script src=\"https://oss.maxcdn.com/html5shiv/3.7.3/html5shiv.min.js\"></script>\n");
  message += F("  <script src=\"https://oss.maxcdn.com/respond/1.4.2/respond.min.js\"></script>\n");
  message += F("<![endif]-->\n");
  message += F("</head>\n");

  message += F("<body>\n");
  message += F("<div class=\"container\">\n");
  message += F("<div class=\"page-header\"><h1>Wetterstation</h1></div>\n");

  message += F("<div id=\"error\"></div>");

  message += F("<div class=\"panel panel-info\"><div class=\"panel-heading\"><h3 class=\"panel-title\">Innen</h3></div><div class=\"panel-body\"><ul>\n");
  message += F("<li>Temperatur: <span id=\"t_in\">");
  message +=  temperature_indoor;
  message += F("</span>&deg;C</li>\n");
  message += F("<li>Relative Feuchtigkeit: <span id=\"h_in\">");
  message +=  int(humidity_indoor);
  message += F("</span>%</li>\n");
  message += F("<li>Absolute Feuchtigkeit: <span id=\"f_in\">");
  message +=  humidity_abs_indoor;
  message += F("</span> g/m³</li>\n");
  message += F("<li>Taupunkt: <span id=\"dp_in\">");
  message += dp_indoor;
  message += F("</span>&deg;C</li>\n");
  message += F("</ul></div></div>\n");

  message += F("<div class=\"panel panel-info\"><div class=\"panel-heading\"><h3 class=\"panel-title\">Aussen</h3></div><div class=\"panel-body\"><ul>\n");
  message += F("<li>Temperatur: <span id=\"t_out\">");
  message +=  temperature_outdoor;
  message += F("</span>&deg;C</li>\n");
  message += F("<li>Relative Feuchtigkeit: <span id=\"h_out\">");
  message +=  int(humidity_outdoor);
  message += F("</span>%</li>\n");
  message += F("<li>Absolute Feuchtigkeit: <span id=\"f_out\">");
  message +=  humidity_abs_outdoor;
  message += F("</span> g/m³</li>\n");
  message += F("</ul></div></div>");

  message += F("<div class=\"panel panel-primary\"><div class=\"panel-heading\"><h3 class=\"panel-title\">Absolute Feuchtigkeitsdifferenz Innen/Aussen</h3></div><div class=\"panel-body\"><ul><li><span id=\"f_diff\">\n");
  message += humidity_abs_indoor - humidity_abs_outdoor;
  message += F("</span> g/m³</li></ul></div></div>\n");

  message += F("</div><!-- /container -->\n");
  message += F("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js\"></script>\n");
  message += F("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\"></script>\n");

  message += F("<script src=\"https://unpkg.com/axios/dist/axios.min.js\"></script>");

  message += F("<script>\n");
  message += F("  Number.prototype.round = function(decimals) { return Number((Math.round(this + \"e\" + decimals)  + \"e-\" + decimals)); }\n");
  message += F("  var fn = function(){\n");
  message += F("    axios.get('/data.json')\n");
  message += F("      .then(function (response) {\n");
  message += F("          document.getElementById('t_in').innerHTML = response.data.t_in.round(1);\n");
  message += F("          document.getElementById('h_in').innerHTML = response.data.h_in;\n");
  message += F("          document.getElementById('f_in').innerHTML = response.data.f_in.round(1);\n");
  message += F("          document.getElementById('dp_in').innerHTML = response.data.dp_in.round(1);\n");
  message += F("          document.getElementById('t_out').innerHTML = response.data.t_out.round(1);\n");
  message += F("          document.getElementById('h_out').innerHTML = response.data.h_out;\n");
  message += F("          document.getElementById('f_out').innerHTML = response.data.f_out.round(1);\n");
  message += F("          document.getElementById('f_diff').innerHTML = response.data.f_diff.round(1);\n");
  message += F("          document.getElementById('error').innerHTML = '';\n");
  message += F("       })\n");
  message += F("      .catch(function (err) {\n");
  message += F("        document.getElementById('error').innerHTML = '<div class=\"alert alert-danger\" role=\"alert\">' + err.message + '</div>';\n");
  message += F("      });\n");
  message += F("      };\n");
  message += F("      fn();\n");
  message += F("      var interval = setInterval(fn, 5000);\n");
  message += F("</script>\n");

  message += F("</body></html>");

  HTTP.send(200, "text/html", message);
}

ICACHE_RAM_ATTR void getRemoteTempHumi(byte * data) {

  if (millis() - receive_beginWait < MIN_RECEIVE_INTERVAL) {
    return;
  }

  // is data a ThermoHygro-device?
  if ((data[3] & 0x1f) == 0x1e) {

    byte channel, randomId;
    int temp;
    byte humidity;

    SensorReceiver::decodeThermoHygro(data, channel, randomId, temp, humidity);

    if (randomId == RF_RECEIVER_ID) {
      humidity_outdoor = humidity;
      temperature_outdoor = temp / 10.0;

      if (temperature_outdoor > -273 && humidity_outdoor > 0) {
        humidity_abs_outdoor = berechneTT(temperature_outdoor, humidity_outdoor);
      }

      DEBUG("Temperature: %u.%u deg, Humidity: %u % REL, ID: %u\n", temp / 10, abs(temp % 10), humidity, randomId);

      receive_beginWait = millis();
    } else {
      DEBUG("ROGUE SENSOR: Temperature: %u.%u deg, Humidity: %u % REL, ID: %u\n", temp / 10, abs(temp % 10), humidity, randomId);
    }
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

void printNumI(int num)
{
  char st[27];

  sprintf(st, "%i", num);
  if (strlen(st) == 1) {
    display.print(" ");
  }
  display.print(st);
}

void printNumF (double num, byte dec, int length)
{
  char st[27];

  dtostrf(num, length, dec, st );
  int l = strlen(st);
  // fixed output length 4
  if (l == 3) {
    display.print(F("  "));
  }
  if (l == 4) {
    display.print(F(" "));
  }
  display.print(st);
}


