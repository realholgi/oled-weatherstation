#include <Wire.h>
#include "WeatherOled.h"
#include "config.h"

WeatherOled::WeatherOled(int8_t rst) : Adafruit_SSD1305(128, 64, &Wire, rst) {}

// Sends all 8 pages via I2C with column start = 2 (hardware-specific offset).
// The official library defaults to column 0, which misaligns this display.
void WeatherOled::display() {
    const uint8_t prefix = 0x40; // I2C data control byte
    i2c_dev->setSpeed(i2c_preclk);   // boost to 400 kHz for pixel writes
    for (uint8_t page = 0; page < 8; page++) {
        const uint8_t cmds[] = {uint8_t(SSD1305_SETPAGESTART + page), 0x02, 0x10};
        oled_commandList(cmds, sizeof(cmds));
        uint8_t *ptr = buffer + (uint16_t)page * 128;
        for (uint8_t chunk = 0; chunk < 8; chunk++) {
            i2c_dev->write(ptr + chunk * 16, 16, true, &prefix, 1);
        }
    }
    i2c_dev->setSpeed(i2c_postclk);  // restore to 100 kHz
    window_x1 = 1024; window_y1 = 1024;
    window_x2 = -1;   window_y2 = -1;
}
