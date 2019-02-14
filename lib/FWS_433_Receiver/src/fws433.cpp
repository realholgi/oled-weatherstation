#include "fws433.h"
#include <stdio.h>
#include <stdlib.h>

#define FILTER_MIN 1500
#define FILTER_MAX 9000
#define MEDIUM_LEN 3000
#define STOP_MIN 7500

//#define dbg(s) Serial.println(s)
#define dbg(s)

const int FWS433::_BUFF_SIZE = 40;

volatile bool FWS433::_avail = false;
volatile byte FWS433::_buff[FWS433::_BUFF_SIZE];
volatile byte FWS433::_buffEnd = 0;

unsigned long FWS433::_lastPackageArrived;
byte FWS433::_lastBuff[FWS433::_BUFF_SIZE];
byte FWS433::_pin = 0;

FWS433::FWS433() {
}

void FWS433::start(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT);
    for (int i = 0; i < _BUFF_SIZE; i++) {
        _buff[i] = 8;
        _lastBuff[i] = 8;
    }
    _lastPackageArrived = 0;
    _avail = false;
    _buffEnd = 0;
    attachInterrupt(digitalPinToInterrupt(_pin), _handler, CHANGE);
    dbg("fws started");
}

void FWS433::stop() {
    detachInterrupt(digitalPinToInterrupt(_pin));
}

bool FWS433::isDataAvailable() {
    return _avail;
}

void FWS433::_handler() {
    static unsigned long lastMs = 0, currMs, diffMs;
    currMs = micros();
    diffMs = currMs - lastMs;
    lastMs = currMs;

    if (diffMs > FILTER_MIN && diffMs < FILTER_MAX) { //Filter out the too long and too short pulses
        if (!_avail) { //avail means data available for processing
            if (diffMs > STOP_MIN) { // INIT/STOP pulse
                dbg("S");
                if (_buffEnd == 40) { //There is the right amount of data in buff
                    if (!_isRepeat()) { //if this is the repeat of the previous package ( in 3 sec) then don't respond a false positive availability.
                        _avail = true;
                    } else {
                        _buffEnd = 0;
                    }
                } else {
                    dbg("S buffEnd:" + String(_buffEnd));
                    _buffEnd = 0;
                }
            } else {
                if (_buffEnd < _BUFF_SIZE) {  //buffer is not full yet
                    if (diffMs < MEDIUM_LEN) { //0
                        _buff[_buffEnd++] = 0;
                        dbg("0");
                    } else { //1
                        _buff[_buffEnd++] = 1;
                        dbg("1");
                    }
                }
            }
        }
    }
}

bool FWS433::_isRepeat() {
    bool result = false;
    for (int i = 0; i < _buffEnd; i++) {
        if (_buff[i] != _lastBuff[i]) {
            for (int j = 0; j < _buffEnd; j++) {
                _lastBuff[j] = _buff[j];
            }
            _lastPackageArrived = millis();
            return false;
        }
    }
    result = (millis() - _lastPackageArrived < 3000);
    _lastPackageArrived = millis();
    return result;
}

// Bit 0 - 7: ID (refreshed on power cycle)
// Bit 8 - 11: CRC? (no simple Nibble-Summing though)
// Bit 12: Transmit botton (1) / Automatic send (0)
// Bit 13: Battery, U < 2.50 V sets bit
// Bit 14: Temp descending
// Bit 15: Temp rising
// Bit 16-27: Temp °F (as known)
// Bit 28-35: Hum % (as known)
// Bit 36-37: Always 00 - unkown
// Bit 38-39: Channel (1-3)


void FWS433::getData(byte &id, byte &channel, byte &humidity, int &temperature, bool &battery) {
    int crc = 0;
    int humi1 = 0, humi2 = 0;

    for (int i = 0; i < 34; i++) {
        if (_buff[i] != (crc & 1)) {
            crc = (crc >> 1) ^ 12;
        } else {
            crc = (crc >> 1);
        }
    }

    dbg("CRCin: " + String(crc));

    crc = _binToDec(_buff, 8, 11);
    dbg("CRCsoll: " + String(crc));

    //crc ^= _binToDec(_buff, 34, 37);
    //if (crc != _binToDec(_buff, 38, 41)) {
    //	dbg("CRC error!");
    //_buffEnd = 0;
    //_avail = false;
    //return;
    //}
    //dbg("CRC OK.");

    int is_transmit_button = _binToDec(_buff, 12, 12);
    int is_temp_descending = _binToDec(_buff, 14, 14);
    int is_temp_rising = _binToDec(_buff, 15, 15);

    humi1 = _binToDecRev(_buff, 28, 31);
    humi2 = _binToDecRev(_buff, 32, 35);
    humidity = humi1 * 10 + humi2;

    if (humidity < 1 || humidity > 100) {
        _buffEnd = 0;
        _avail = false;
        return;
    }

    id = _binToDecRev(_buff, 0, 7);
    channel = _binToDecRev(_buff, 38, 39);

    temperature = _binToDecRev(_buff, 16, 27);

    // temperature = (int)((float)(((temperature*10) - 9000) - 3200) * ((float)5/(float)9));
    temperature = (temperature / 10) - 90.0;
    temperature = (temperature - 32) / 1.8 * 10;


    battery = _binToDecRev(_buff, 13, 13) != 1;
    _avail = false;
}

fwsResult FWS433::getData() {
    fwsResult result;
    getData(result.id, result.channel, result.humidity, result.temperature, result.battery);
    return result;
}

int FWS433::_binToDecRev(volatile byte *binary, int s, int e) {
    int result = 0;
    unsigned int mask = 1;
    for (; e > 0 && s <= e; mask <<= 1)
        if (binary[e--] != 0)
            result |= mask;
    return result;
}

int FWS433::_binToDec(volatile byte *binary, int s, int e) {
    unsigned int mask = 1;
    int result = 0;
    for (; s <= e; mask <<= 1)
        if (binary[s++] != 0)
            result |= mask;
    return result;
}
