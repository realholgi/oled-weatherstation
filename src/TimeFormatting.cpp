#include "TimeFormatting.h"
#include <cstdio>

namespace TimeFormatting {

void formatClock(char *buffer, size_t bufferSize, const std::tm *localTimeInfo) {
    if (!buffer || bufferSize == 0) return;
    if (!localTimeInfo) {
        snprintf(buffer, bufferSize, "--:--");
        return;
    }
    snprintf(buffer, bufferSize, "%02d:%02d", localTimeInfo->tm_hour, localTimeInfo->tm_min);
}

}
