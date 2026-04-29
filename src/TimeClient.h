#pragma once

#include <Arduino.h>

class TimeClient {

private:
    const float myUtcOffset;
    const long timezoneOffsetSeconds;
    long localEpoc = 0;
    unsigned long localMillisAtUpdate = 0;
    bool timeSet = false;

public:
    TimeClient(float utcOffset);

    void updateTime();

    void getFormattedTime(char *buffer, size_t bufferSize);

private:
    long getCurrentEpoch();
    long getCurrentEpochWithUtcOffset();
    int convertMonthNameToNumber(const String &strMonthName);
};
