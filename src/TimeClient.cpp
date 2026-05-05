#include "TimeClient.h"
#include <Arduino.h>
#include <time.h>
#include <cstdio>

void TimeClient::configure(const char *timezonePosix, const char *ntpServer) {
    configTzTime(timezonePosix, ntpServer);
}

bool TimeClient::isTimeSet() const {
    return time(nullptr) > 1000000000UL;
}

void TimeClient::getFormattedTime(char *buffer, size_t bufferSize) const {
    if (!buffer || bufferSize == 0) return;
    if (!isTimeSet()) {
        snprintf(buffer, bufferSize, "--:--");
        return;
    }
    time_t currentTime = time(nullptr);
    struct tm *localTimeInfo = localtime(&currentTime);
    snprintf(buffer, bufferSize, "%02d:%02d", localTimeInfo->tm_hour, localTimeInfo->tm_min);
}
