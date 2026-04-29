#include <Arduino.h>
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "SensorSanity.h"
#include "TimeClient.h"
#include "config.h"
#include "icons.h"

Display::Display() : oled(OLED_RESET) {}

void Display::setup() {
    oled.begin();
    oled.setRotation(3);
    oled.clearDisplay();
    oled.display();
    oled.setTextWrap(false);
    oled.setTextColor(WHITE);
    oled.setTextSize(1);
}

void Display::showSensorFailure() {
    clear();
    printAt(6, 0, "ERROR:", false);
    printAt(6, 20, "local", false);
    printAt(6, 30, "Sensor", false);
    printAt(6, 40, "failing!");
}

void Display::showStartupConfig() {
    showStartupStage(0, "Config...");
}

void Display::showStartupWifi() {
    showStartupStage(10, "WIFI");
}

void Display::showStartupHttp() {
    showStartupStage(20, "HTTP...");
}

void Display::showStartupTime() {
    showStartupStage(30, "Time...");
}

void Display::showStartupOutdoorSensor() {
    showStartupStage(40, "433MHz...");
}

void Display::showConfigPortalNoCredentials() {
    clear();
    printAt(6, 0, "no Cfg", false);
    printAt(6, 10, "Portal");
}

void Display::showConfigPortalReset() {
    clear();
    printAt(6, 0, "RESET", false);
    printAt(6, 10, "Portal");
}

void Display::showConfigPortalSsid(const String &ssid) {
    clear();
    setTextSize(1);
    printAt(6, 0, "SETUP", false);
    printAt(6, 20, "SSID:", false);
    printAt(6, 40, ssid);
}

void Display::appendWifiProgress() {
    print(".");
    flush();
}

void Display::printAt(int x, int y, const char *st, boolean onScreen) {
    oled.setCursor(x, y + OFFSET);
    oled.print(st);
    if (onScreen) oled.display();
}

void Display::printAt(int x, int y, const String &st, boolean onScreen) {
    oled.setCursor(x, y + OFFSET);
    oled.print(st);
    if (onScreen) oled.display();
}

void Display::clear() {
    oled.clearDisplay();
}

void Display::setTextSize(uint8_t size) {
    oled.setTextSize(size);
}

void Display::print(const char *st) {
    oled.print(st);
}

void Display::flush() {
    oled.display();
}

void Display::showStartupStage(int y, const char *message) {
    printAt(6, y, message);
}

void Display::printNumI(int x, int y, int num) {
    char st[27];
    oled.setCursor(x, y + OFFSET);
    snprintf(st, sizeof(st), "%i", num);
    if (strlen(st) == 1) oled.print(" ");
    oled.print(st);
}

void Display::printNumF(int x, int y, double num, byte dec, int length) {
    char st[27];
    oled.setCursor(x, y + OFFSET);
    dtostrf(num, length, dec, st);
    int l = strlen(st);
    if (l == 3) oled.print(F("  "));
    if (l == 4) oled.print(F(" "));
    oled.print(st);
}

void Display::renderData(TimeClient &timeClient, const SensorIndoor &indoorSensor,
                         const SensorOutdoor &outdoorSensor) {
    char formattedTime[6];
    timeClient.getFormattedTime(formattedTime, sizeof(formattedTime));

    oled.clearDisplay();
    oled.setTextSize(2);

    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature())) {
        printNumF(6, 0, outdoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(outdoorSensor.humidity())) {
        printNumI(46, 20, outdoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleHumidity(outdoorSensor.absoluteHumidity())) {
        printNumF(34, 30, outdoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, 20 + OFFSET, sun_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, 40 + OFFSET, oled.width() - 1, 40 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(indoorSensor.temperature())) {
        printNumF(6, 40 + 4, indoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        printNumI(46, 20 + 40 + 2, indoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleHumidity(indoorSensor.absoluteHumidity())) {
        printNumF(34, 30 + 40 + 2, indoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, 20 + 40 + 2 + OFFSET, home_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, 82 + OFFSET, oled.width() - 1, 82 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(outdoorSensor.humidity()) &&
        SensorSanity::isPlausibleTemperature(indoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        float diff = indoorSensor.absoluteHumidity() - outdoorSensor.absoluteHumidity();
        if (abs(diff) < 0.05f) {
            diff = 0.0f;
        }
        printNumF(6, 82 + 4, diff);
        int color = WHITE;
        if (diff > 0.0 && abs(diff) < MIN_DIFF) color = BLACK;
        oled.drawBitmap(0, 3 + 82 + OFFSET, warning_icon16x16, 16, 16, color);
    }

    printAt(6, 82 + 4 + 22, formattedTime);
}
