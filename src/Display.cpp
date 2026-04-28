#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "Display.h"
#include "SensorIndoor.h"
#include "SensorOutdoor.h"
#include "TimeClient.h"
#include "icons.h"

Adafruit_SSD1305 display(OLED_RESET);

void setupDisplay() {
    display.begin();
    display.setRotation(3);
    display.clearDisplay();
    display.display();
    display.setTextWrap(false);
    display.setTextColor(WHITE);
    display.setTextSize(1);
}

void printAt(int x, int y, const char *st, boolean onScreen) {
    display.setCursor(x, y + OFFSET);
    display.print(st);
    if (onScreen) display.display();
}

void printAt(int x, int y, String st, boolean onScreen) {
    display.setCursor(x, y + OFFSET);
    display.print(st);
    if (onScreen) display.display();
}

void printNumI(int x, int y, int num) {
    char st[27];
    display.setCursor(x, y + OFFSET);
    snprintf(st, sizeof(st), "%i", num);
    if (strlen(st) == 1) display.print(" ");
    display.print(st);
}

void printNumF(int x, int y, double num, byte dec, int length) {
    char st[27];
    display.setCursor(x, y + OFFSET);
    dtostrf(num, length, dec, st);
    int l = strlen(st);
    if (l == 3) display.print(F("  "));
    if (l == 4) display.print(F(" "));
    display.print(st);
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

    if (humidity_abs_outdoor > -1) {
        printNumF(34, 30, humidity_abs_outdoor);
    }

    display.drawBitmap(2, 20 + OFFSET, sun_icon16x16, 16, 16, WHITE);
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

    if (humidity_abs_indoor > -1) {
        printNumF(34, 30 + 40 + 2, humidity_abs_indoor);
    }

    display.drawBitmap(2, 20 + 40 + 2 + OFFSET, home_icon16x16, 16, 16, WHITE);
    display.drawLine(0, 82 + OFFSET, display.width() - 1, 82 + OFFSET, WHITE);

    display.setTextSize(2);
    if (temperature_outdoor > -273 && humidity_outdoor > 0 && temperature_indoor > -273 && humidity_indoor > 0) {
        float diff = humidity_abs_indoor - humidity_abs_outdoor;
        if (abs(diff) < 0.05f) {
            diff = 0.0f;
        }
        printNumF(6, 82 + 4, diff);
        int color = WHITE;
        if (diff > 0.0 && abs(diff) < MIN_DIFF) color = BLACK;
        display.drawBitmap(0, 3 + 82 + OFFSET, warning_icon16x16, 16, 16, color);
    }

    char timeBuffer[6];
    timeClient.getFormattedTime(timeBuffer, sizeof(timeBuffer));
    printAt(6, 82 + 4 + 22, timeBuffer);
}
