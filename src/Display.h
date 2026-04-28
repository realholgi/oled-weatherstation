#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1305.h>

extern Adafruit_SSD1305 display;

void setupDisplay();
void displayData();
void printAt(int x, int y, const char *st, boolean onScreen = true);
void printAt(int x, int y, String st, boolean onScreen = true);
void printNumI(int x, int y, int num);
void printNumF(int x, int y, double num, byte dec = 1, int length = 0);
