#include "fws433.h"

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

IRAM_ATTR void FWS433::_handler() {
    static unsigned long lastMs = 0;
    unsigned long currMs = micros();
    unsigned long diffMs = currMs - lastMs;
    lastMs = currMs;

    if (_avail || diffMs <= FILTER_MIN || diffMs >= FILTER_MAX) {
        return;
    }

    if (diffMs > STOP_MIN) { // INIT/STOP pulse
        if (_buffEnd == 40) {
            if (_isRepeat()) {
                _avail = true;
            }
        }
        _buffEnd = 0;
    } else if (_buffEnd < _BUFF_SIZE) {
        _buff[_buffEnd++] = (diffMs >= MEDIUM_LEN);
    }
}

IRAM_ATTR bool FWS433::_isRepeat() {
    for (int i = 0; i < _buffEnd; i++) {
        if (_buff[i] != _lastBuff[i]) {
            for (int j = 0; j < _buffEnd; j++) {
                _lastBuff[j] = _buff[j];
            }
            _lastPackageArrived = millis();
            return false;
        }
    }
    bool result = (millis() - _lastPackageArrived < 3000);
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
    id = _binToDecRev(_buff, 0, 7);
    battery = _buff[13] != 1;
    temperature = _binToDecRev(_buff, 16, 27);
    temperature = (temperature - 1220) * 5 / 9;

    byte h1 = _binToDecRev(_buff, 28, 31);
    byte h2 = _binToDecRev(_buff, 32, 35);
    humidity = h1 * 10 + h2;

    if (humidity < 1 || humidity > 100) {
        _buffEnd = 0;
        _avail = false;
        return;
    }

    channel = _binToDecRev(_buff, 38, 39);
    _buffEnd = 0;
    _avail = false;
}

fwsResult FWS433::getData() {
    fwsResult result;
    getData(result.id, result.channel, result.humidity, result.temperature, result.battery);
    return result;
}

int FWS433::_binToDecRev(const volatile byte *binary, int s, int e) const {
    int result = 0;
    for (int i = s; i <= e; i++) {
        result = (result << 1) | (binary[i] & 1);
    }
    return result;
}

int FWS433::_binToDec(const volatile byte *binary, int s, int e) const {
    int result = 0;
    for (int i = e; i >= s; i--) {
        result = (result << 1) | (binary[i] & 1);
    }
    return result;
}
