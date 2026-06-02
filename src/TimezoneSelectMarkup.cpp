#ifdef ARDUINO

#include "TimezoneSelectMarkup.h"
#include <string.h>

namespace TimezoneSelectMarkup {

String build(const TzEntry* entries, size_t count, const char* currentPosix) {
    bool hasMatch = false;
    for (size_t i = 0; i < count && !hasMatch; i++) {
        hasMatch = strcmp(entries[i].posix, currentPosix) == 0;
    }
    String result;
    result.reserve(capacity(entries, count, hasMatch));
    result = "<br/><label>Timezone</label>"
             "<select onchange=\"document.getElementById('timezone_posix').value=this.value\">";
    for (size_t i = 0; i < count; i++) {
        result += "<option value='";
        result += entries[i].posix;
        result += "'";
        if (strcmp(entries[i].posix, currentPosix) == 0) result += " selected";
        result += ">";
        result += entries[i].name;
        result += "</option>";
    }
    result += "</select>";
    return result;
}

String buildLanguage(const char* currentLanguage) {
    // Language select always produces exactly LANGUAGE_SELECT_CAPACITY (200) bytes
    // regardless of which language is selected, because exactly one option always
    // carries " selected". Formula: 112 (open) + 70 (both options base) + 9 (close) + 9 (selected)
    const bool isEnglish = strcmp(currentLanguage, "en") == 0;
    String result;
    result.reserve(LANGUAGE_SELECT_CAPACITY);
    result = "<br/><label>Webpage Language</label>"
             "<select onchange=\"document.getElementById('web_language').value=this.value\">";
    result += "<option value='de'";
    if (!isEnglish) result += " selected";
    result += ">Deutsch</option>";
    result += "<option value='en'";
    if (isEnglish) result += " selected";
    result += ">English</option>";
    result += "</select>";
    return result;
}

}  // namespace TimezoneSelectMarkup

#endif  // ARDUINO
