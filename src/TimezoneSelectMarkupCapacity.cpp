#include "TimezoneSelectMarkup.h"
#include <string.h>

namespace TimezoneSelectMarkup {

size_t capacity(const TzEntry* entries, size_t count, bool hasMatch) {
    const size_t FIXED     = 106 + 9;   // open wrapper (106) + "</select>" (9)
    const size_t PER_ENTRY = 26;        // "<option value='" + "'" + ">" + "</option>"
    const size_t SELECTED  = 9;         // " selected"
    size_t total = FIXED;
    for (size_t i = 0; i < count; i++) {
        total += PER_ENTRY + strlen(entries[i].posix) + strlen(entries[i].name);
    }
    if (hasMatch) total += SELECTED;
    return total;
}

}  // namespace TimezoneSelectMarkup
