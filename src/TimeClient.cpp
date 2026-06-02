#include "TimeClient.h"
#include "TimeFormatting.h"
#include <Arduino.h>
#include <time.h>

void TimeClient::configure(const char *timezonePosix, const char *ntpServer) {
    ntpServerStorage = ntpServer;  // sntp_setservername stores raw ptr, not a copy
    configTzTime(timezonePosix, ntpServerStorage.c_str());
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
    TimeFormatting::formatClock(buffer, bufferSize, localTimeInfo);
}
