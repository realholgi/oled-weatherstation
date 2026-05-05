#include <Arduino.h>
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "SensorSanity.h"
#include "TimeClient.h"
#include "config.h"
#include "icons.h"

Display::Display() : oled(OLED_RESET) {}

void Display::begin() {
    oled.begin();
    oled.setRotation(3);
    oled.clearDisplay();
    oled.display();
    oled.setTextWrap(false);
    oled.setTextColor(WHITE);
    oled.setTextSize(1);
}

void Display::showSensorFailure() {
    oled.clearDisplay();
    drawTextAt(6, 0, "ERROR:", false);
    drawTextAt(6, 20, "local", false);
    drawTextAt(6, 30, "Sensor", false);
    drawTextAt(6, 40, "failing!");
}

void Display::showStartupConfig() {
    drawTextAt(6, 0, "Config...");
}

void Display::showStartupWifi() {
    drawTextAt(6, 10, "WIFI...");
}

void Display::showStartupHttp() {
    drawTextAt(6, 20, "HTTP...");
}

void Display::showStartupTime() {
    drawTextAt(6, 30, "Time...");
}

void Display::showStartupOutdoorSensor() {
    drawTextAt(6, 40, "433MHz...");
}

void Display::showConfigPortalNoCredentials() {
    oled.clearDisplay();
    drawTextAt(6, 0, "no Cfg", false);
    drawTextAt(6, 10, "Portal");
}

void Display::showConfigPortalReset() {
    oled.clearDisplay();
    drawTextAt(6, 0, "RESET", false);
    drawTextAt(6, 10, "Portal");
}

void Display::showConfigPortalSsid(const String &ssid) {
    oled.clearDisplay();
    oled.setTextSize(1);
    drawTextAt(6, 0, "SETUP", false);
    drawTextAt(6, 20, "SSID:", false);
    drawTextAt(6, 40, ssid);
}

void Display::appendWifiConnectionProgress() {
    oled.print(".");
    oled.display();
}

void Display::drawTextAt(int x, int y, const char *text, boolean updateDisplay) {
    oled.setCursor(x, y + OFFSET);
    oled.print(text);
    if (updateDisplay) oled.display();
}

void Display::drawTextAt(int x, int y, const String &text, boolean updateDisplay) {
    oled.setCursor(x, y + OFFSET);
    oled.print(text);
    if (updateDisplay) oled.display();
}

void Display::drawIntegerAt(int x, int y, int value) {
    char textBuffer[27];
    oled.setCursor(x, y + OFFSET);
    snprintf(textBuffer, sizeof(textBuffer), "%i", value);
    if (strlen(textBuffer) == 1) oled.print(" ");
    oled.print(textBuffer);
}

void Display::drawFloatAt(int x, int y, double value, byte decimals, int minimumWidth) {
    char textBuffer[27];
    oled.setCursor(x, y + OFFSET);
    dtostrf(value, minimumWidth, decimals, textBuffer);
    int textLength = strlen(textBuffer);
    if (textLength == 3) oled.print(F("  "));
    if (textLength == 4) oled.print(F(" "));
    oled.print(textBuffer);
}

void Display::renderMeasurements(TimeClient &timeClient, const SensorIndoor &indoorSensor,
                                 const SensorOutdoor &outdoorSensor) {
    char formattedTime[6];
    timeClient.getFormattedTime(formattedTime, sizeof(formattedTime));

    oled.clearDisplay();
    oled.setTextSize(2);

    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature())) {
        drawFloatAt(6, 0, outdoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(outdoorSensor.humidity())) {
        drawIntegerAt(46, 20, outdoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleHumidity(outdoorSensor.absoluteHumidity())) {
        drawFloatAt(34, 30, outdoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, 20 + OFFSET, sun_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, 40 + OFFSET, oled.width() - 1, 40 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(indoorSensor.temperature())) {
        drawFloatAt(6, 40 + 4, indoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        drawIntegerAt(46, 20 + 40 + 2, indoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleHumidity(indoorSensor.absoluteHumidity())) {
        drawFloatAt(34, 30 + 40 + 2, indoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, 20 + 40 + 2 + OFFSET, home_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, 82 + OFFSET, oled.width() - 1, 82 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(outdoorSensor.humidity()) &&
        SensorSanity::isPlausibleTemperature(indoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        float absoluteHumidityDifference = indoorSensor.absoluteHumidity() - outdoorSensor.absoluteHumidity();
        if (abs(absoluteHumidityDifference) < 0.05f) {
            absoluteHumidityDifference = 0.0f;
        }
        drawFloatAt(6, 82 + 4, absoluteHumidityDifference);
        int color = WHITE;
        if (absoluteHumidityDifference > 0.0 && abs(absoluteHumidityDifference) < MIN_DIFF) color = BLACK;
        oled.drawBitmap(0, 3 + 82 + OFFSET, warning_icon16x16, 16, 16, color);
    }

    drawTextAt(6, 82 + 4 + 22, formattedTime);
}
