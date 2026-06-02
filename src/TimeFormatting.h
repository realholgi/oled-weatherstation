#pragma once

#include <cstddef>
#include <ctime>

namespace TimeFormatting {
void formatClock(char *buffer, size_t bufferSize, const std::tm *localTimeInfo);
}
