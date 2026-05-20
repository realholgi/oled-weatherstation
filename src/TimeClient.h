#pragma once

#include <Arduino.h>
#include <cstddef>

class TimeClient {
public:
    void configure(const char *timezonePosix, const char *ntpServer);
    bool isTimeSet() const;
    void getFormattedTime(char *buffer, size_t bufferSize) const;

private:
    String ntpServerStorage; // sntp_setservername stores raw ptr; this keeps it alive
};
