#include "TimeClient.h"
#include <Arduino.h>
#include <time.h>
#include <cstdio>

void TimeClient::configure(const char *tzPosix, const char *ntpServer) {
    configTzTime(tzPosix, ntpServer);
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
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    snprintf(buffer, bufferSize, "%02d:%02d", t->tm_hour, t->tm_min);
}
