#include "TimeClient.h"
#include "WetterDebug.h"
#include "DSTEurope.h"

#include <ESP8266WiFi.h>
#include <cmath>
#include <cstdio>

TimeClient::TimeClient(float utcOffset)
    : timezoneOffsetSeconds(lround(3600.0f * utcOffset)) {
}

void TimeClient::updateTime() {
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect("google.com", httpPort)) {
        DEBUG_MSG("connection failed\n");
        return;
    }

    client.print(String("GET / HTTP/1.1\r\n") +
                 String("Host: google.com\r\n") +
                 String("Connection: close\r\n\r\n"));
    int repeatCounter = 0;
    while (!client.available() && repeatCounter < 10) {
        delay(1000);
        repeatCounter++;
    }

    String line;

    int size = 0;
    client.setNoDelay(false);
    while (client.available() || client.connected()) {
        while ((size = client.available()) > 0) {
            line = client.readStringUntil('\n');
            line.toUpperCase();
            // example:
            // date: Thu, 19 Nov 2015 20:25:40 GMT
            if (line.startsWith("DATE: ")) {
                int parsedHours = line.substring(23, 25).toInt();
                int parsedMinutes = line.substring(26, 28).toInt();
                int parsedSeconds = line.substring(29, 31).toInt();

                int parsedYear = line.substring(18, 22).toInt();
                String strparsedMonth = line.substring(14, 17);
                int parsedMonth = convertMonthNameToNumber(strparsedMonth);
                int parsedDay = line.substring(11, 13).toInt();

                parsedHours = parsedHours + DSTEurope::adjust(parsedYear, parsedMonth, parsedDay, parsedHours);

                localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
                localMillisAtUpdate = millis();
                timeSet = true;
            }
        }
    }

}

void TimeClient::getFormattedTime(char *buffer, size_t bufferSize) {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }

    if (!timeSet) {
        snprintf(buffer, bufferSize, "--:--");
        return;
    }

    const long currentEpoch = getCurrentEpochWithUtcOffset();
    const int hours = static_cast<int>((currentEpoch % 86400L) / 3600L);
    const int minutes = static_cast<int>((currentEpoch % 3600L) / 60L);
    snprintf(buffer, bufferSize, "%02d:%02d", hours, minutes);
}

long TimeClient::getCurrentEpoch() {
    return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

long TimeClient::getCurrentEpochWithUtcOffset() {
    long adjustedEpoch = getCurrentEpoch() + timezoneOffsetSeconds;
    adjustedEpoch %= 86400L;
    if (adjustedEpoch < 0) {
        adjustedEpoch += 86400L;
    }
    return adjustedEpoch;
}

int TimeClient::convertMonthNameToNumber(const String &strMonthName) {
    if (strMonthName == "JAN") { return 1; }
    else if (strMonthName == "FEB") { return 2; }
    else if (strMonthName == "MAR") { return 3; }
    else if (strMonthName == "APR") { return 4; }
    else if (strMonthName == "MAY") { return 5; }
    else if (strMonthName == "JUN") { return 6; }
    else if (strMonthName == "JUL") { return 7; }
    else if (strMonthName == "AUG") { return 8; }
    else if (strMonthName == "SEP") { return 9; }
    else if (strMonthName == "OCT") { return 10; }
    else if (strMonthName == "NOV") { return 11; }
    else if (strMonthName == "DEC") { return 12; }
    return 0;
}
