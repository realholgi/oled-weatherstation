#pragma once

#include <ESP8266WiFi.h>

class TimeClient {

private:
    float myUtcOffset = 0;
    long localEpoc = 0;
    unsigned long localMillisAtUpdate = 0;

public:
    TimeClient(float utcOffset);

    void updateTime();

    void setUtcOffset(float utcOffset);

    void getFormattedTime(char *buffer, size_t bufferSize);

    long getCurrentEpoch();

    long getCurrentEpochWithUtcOffset();

private:
    int convertMonthNameToNumber(String strMonthName);
};

extern TimeClient timeClient;
