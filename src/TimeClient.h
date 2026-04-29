#pragma once

#include <cstddef>

class TimeClient {
public:
    void configure(const char *tzPosix, const char *ntpServer);
    bool isTimeSet() const;
    void getFormattedTime(char *buffer, size_t bufferSize) const;
};
