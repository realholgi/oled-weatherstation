#include <Arduino.h>
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "SensorSanity.h"
#include "TimeClient.h"
#include "VentingAdvice.h"
#include "config.h"
#include "icons.h"

static constexpr int ROW_OUTDOOR_TEMP = 0;
static constexpr int ROW_OUTDOOR_AUX  = 20;
static constexpr int ROW_OUTDOOR_ABS  = 30;
static constexpr int ROW_DIVIDER1     = 40;
static constexpr int ROW_INDOOR_TEMP  = 44;
static constexpr int ROW_INDOOR_AUX   = 62;
static constexpr int ROW_INDOOR_ABS   = 72;
static constexpr int ROW_DIVIDER2     = 82;
static constexpr int ROW_DIFF         = 86;
static constexpr int ROW_TIME         = 108;

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

void Display::showWifiFailure() {
    oled.clearDisplay();
    drawTextAt(6, 0, "WIFI", false);
    drawTextAt(6, 10, "failed", false);
    drawTextAt(6, 30, "running", false);
    drawTextAt(6, 40, "offline");
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

void Display::setVentingThreshold(float threshold) {
    ventingThreshold = threshold;
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
    drawTextAt(x, y, text.c_str(), updateDisplay);
}

void Display::drawIntegerAt(int x, int y, int value) {
    char textBuffer[27];
    oled.setCursor(x, y + OFFSET);
    snprintf(textBuffer, sizeof(textBuffer), "%2i", value);
    oled.print(textBuffer);
}

void Display::drawFloatAt(int x, int y, double value, byte decimals, int minimumWidth) {
    char textBuffer[27];
    oled.setCursor(x, y + OFFSET);
    snprintf(textBuffer, sizeof(textBuffer), "%*.*f", minimumWidth, decimals, value);
    oled.print(textBuffer);
}

void Display::renderMeasurements(TimeClient &timeClient, const SensorIndoor &indoorSensor,
                                 const SensorOutdoor &outdoorSensor) {
    char formattedTime[6];
    timeClient.getFormattedTime(formattedTime, sizeof(formattedTime));

    oled.clearDisplay();
    oled.setTextSize(2);

    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature())) {
        drawFloatAt(6, ROW_OUTDOOR_TEMP, outdoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(outdoorSensor.humidity())) {
        drawIntegerAt(46, ROW_OUTDOOR_AUX, outdoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleAbsoluteHumidity(outdoorSensor.absoluteHumidity())) {
        drawFloatAt(34, ROW_OUTDOOR_ABS, outdoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, ROW_OUTDOOR_AUX + OFFSET, sun_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, ROW_DIVIDER1 + OFFSET, oled.width() - 1, ROW_DIVIDER1 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(indoorSensor.temperature())) {
        drawFloatAt(6, ROW_INDOOR_TEMP, indoorSensor.temperature());
    }

    oled.setTextSize(1);
    if (SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        drawIntegerAt(46, ROW_INDOOR_AUX, indoorSensor.humidity());
        oled.print("%");
    }

    if (SensorSanity::isPlausibleAbsoluteHumidity(indoorSensor.absoluteHumidity())) {
        drawFloatAt(34, ROW_INDOOR_ABS, indoorSensor.absoluteHumidity());
    }

    oled.drawBitmap(2, ROW_INDOOR_AUX + OFFSET, home_icon16x16, 16, 16, WHITE);
    oled.drawLine(0, ROW_DIVIDER2 + OFFSET, oled.width() - 1, ROW_DIVIDER2 + OFFSET, WHITE);

    oled.setTextSize(2);
    if (SensorSanity::isPlausibleTemperature(outdoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(outdoorSensor.humidity()) &&
        SensorSanity::isPlausibleTemperature(indoorSensor.temperature()) &&
        SensorSanity::isPlausibleHumidity(indoorSensor.humidity())) {
        const VentingAdvice::Result advice = VentingAdvice::calculate(indoorSensor.absoluteHumidity(), outdoorSensor.absoluteHumidity(), ventingThreshold);
        drawFloatAt(6, ROW_DIFF, advice.difference);
        oled.drawBitmap(0, ROW_DIFF - 1 + OFFSET, warning_icon16x16, 16, 16,
                       advice.recommendation == VentingAdvice::Recommendation::VENT ? WHITE : BLACK);
    }

    drawTextAt(6, ROW_TIME, formattedTime);
}
