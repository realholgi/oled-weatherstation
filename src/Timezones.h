#ifndef TIMEZONES_H
#define TIMEZONES_H

#include <cstddef>

struct TzEntry { const char *name; const char *posix; };

extern const TzEntry TIMEZONES[];
extern const size_t TZ_COUNT;

#endif // TIMEZONES_H
