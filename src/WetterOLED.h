#pragma once

#include <Adafruit_SSD1305.h>

// Fixes the column start address to 0x02 (upstream default is 0x00) for this display.
class WetterOLED : public Adafruit_SSD1305 {
public:
    WetterOLED(int8_t rst);
    void display() override;
};
