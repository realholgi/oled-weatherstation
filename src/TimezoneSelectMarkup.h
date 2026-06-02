#pragma once

#include "Timezones.h"
#include <stddef.h>

#ifdef ARDUINO
#include <WString.h>
#endif

namespace TimezoneSelectMarkup {

// Pure — native-testable. No Arduino dependency.
size_t capacity(const TzEntry* entries, size_t count, bool hasMatch);

// Firmware-only. Returns Arduino String — not native-compilable.
#ifdef ARDUINO
String build(const TzEntry* entries, size_t count, const char* currentPosix);
#endif

}  // namespace TimezoneSelectMarkup
