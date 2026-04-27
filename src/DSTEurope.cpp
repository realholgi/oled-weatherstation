#include "DSTEurope.h"

int DSTEurope::adjust(int iYear, int iMonth, int iDay, int utcHour) {
    // last sunday of march
    const int beginDSTDate = (31 - (5 * iYear / 4 + 4) % 7);
    const int beginDSTMonth = 3;
    // last sunday of october
    const int endDSTDate = (31 - (5 * iYear / 4 + 1) % 7);
    const int endDSTMonth = 10;
    if (((iMonth > beginDSTMonth) && (iMonth < endDSTMonth))
        || ((iMonth == beginDSTMonth) && (iDay > beginDSTDate))
        || ((iMonth == beginDSTMonth) && (iDay == beginDSTDate) && (utcHour >= 1))
        || ((iMonth == endDSTMonth) && (iDay < endDSTDate))
        || ((iMonth == endDSTMonth) && (iDay == endDSTDate) && (utcHour < 1)))
        return 1;
    else return 0;
}
