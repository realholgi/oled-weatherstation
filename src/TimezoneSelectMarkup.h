#pragma once

#include "Timezones.h"
#include <stddef.h>

#ifdef ARDUINO
#include <WString.h>
#endif

namespace TimezoneSelectMarkup {

// Pure — native-testable. No Arduino dependency.
size_t capacity(const TzEntry* entries, size_t count, bool hasMatch);

// Language select always produces exactly 200 bytes regardless of selected language.
// Formula: 112 (open wrapper) + 70 (both options without selected) + 9 (close) + 9 (one selected)
constexpr size_t LANGUAGE_SELECT_CAPACITY = 200;

// Firmware-only. Returns Arduino String — not native-compilable.
#ifdef ARDUINO
String build(const TzEntry* entries, size_t count, const char* currentPosix);
String buildLanguage(const char* currentLanguage);
#endif

}  // namespace TimezoneSelectMarkup
