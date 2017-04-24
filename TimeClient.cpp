/**The MIT License (MIT)

  Copyright (c) 2015 by Daniel Eichhorn

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  See more at http://blog.squix.ch
*/

#include "TimeClient.h"

TimeClient::TimeClient(float utcOffset, int adjust_dst)
{
  myUtcOffset = utcOffset;
  adjustDST = adjust_dst;
}

void TimeClient::updateTime()
{
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("www.google.com", httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(1000);
    Serial.println(".");
    repeatCounter++;
  }

  String line;

  int size = 0;
  client.setNoDelay(false);
  while (client.connected()) {
    while ((size = client.available()) > 0) {
      line = client.readStringUntil('\n');
      line.toUpperCase();
      // example:
      // date: Thu, 19 Nov 2015 20:25:40 GMT
      if (line.startsWith("DATE: ")) {
        //Serial.println(line.substring(23, 25) + ":" + line.substring(26, 28) + ":" + line.substring(29, 31));
        int parsedHours = line.substring(23, 25).toInt();
        int parsedMinutes = line.substring(26, 28).toInt();
        int parsedSeconds = line.substring(29, 31).toInt();
        //Serial.println(String(parsedHours) + ":" + String(parsedMinutes) + ":" + String(parsedSeconds));

        int parsedYear = line.substring(18, 22 ).toInt();
        String strparsedMonth =  line.substring(14, 17);
        int parsedMonth = convertMonthNameToNumber(strparsedMonth);
        int parsedDay = line.substring(11, 13).toInt();
        //Serial.println("Annee: " + String(parsedYear) + " mois: " + String(parsedMonth) + " Jour; " + String(parsedDay));

        if (adjustDST == 1) {
          parsedHours = parsedHours + adjustDSTEurope(parsedYear, parsedMonth, parsedDay);
        }

        localEpoc = (parsedHours * 60 * 60 + parsedMinutes * 60 + parsedSeconds);
        //Serial.println(localEpoc);
        localMillisAtUpdate = millis();
      }
    }
  }

}

String TimeClient::getHours()
{
  if (localEpoc == 0) {
    return "--";
  }
  int hours = ((getCurrentEpochWithUtcOffset()  % 86400L) / 3600) % 24;
  if (hours < 10) {
    return "0" + String(hours);
  }
  return String(hours); // print the hour (86400 equals secs per day)

}
String TimeClient::getMinutes()
{
  if (localEpoc == 0) {
    return "--";
  }
  int minutes = ((getCurrentEpochWithUtcOffset() % 3600) / 60);
  if (minutes < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    return "0" + String(minutes);
  }
  return String(minutes);
}

String TimeClient::getSeconds()
{
  if (localEpoc == 0) {
    return "--";
  }
  int seconds = getCurrentEpochWithUtcOffset() % 60;
  if ( seconds < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    return "0" + String(seconds);
  }
  return String(seconds);
}

String TimeClient::getFormattedTime()
{
  return getHours() + ":" + getMinutes() + ":" + getSeconds();
}

long TimeClient::getCurrentEpoch()
{
  return localEpoc + ((millis() - localMillisAtUpdate) / 1000);
}

long TimeClient::getCurrentEpochWithUtcOffset()
{
  return round(getCurrentEpoch() + 3600 * myUtcOffset + 86400L) % 86400L;
}

int TimeClient::convertMonthNameToNumber (String strMonthName)
{
  if (strMonthName == "JAN")
  {
    return 1;
  }
  else if (strMonthName == "FEB")
  {
    return 2;
  }
  else if (strMonthName == "MAR")
  {
    return 3;
  }
  else if (strMonthName == "APR")
  {
    return 4;
  }
  else if (strMonthName == "MAI")
  {
    return 5;
  }
  else if (strMonthName == "JUN")
  {
    return 6;
  }
  else if (strMonthName == "JUL")
  {
    return 7;
  }
  else if (strMonthName == "AUG")
  {
    return 8;
  }
  else if (strMonthName == "SEP")
  {
    return 9;
  }
  else if (strMonthName == "OCT")
  {
    return 10;
  }
  else if (strMonthName == "NOV")
  {
    return 11;
  }
  else if (strMonthName == "DEC")
  {
    return 12;
  }
}

int TimeClient::adjustDSTEurope(int year, int month, int day)
{
  // http://www.webexhibits.org/daylightsaving/i.html
  //
  // Since 1996, valid through 2099

  int t = int(5 * year / 4);

  // last sunday of march
  int startDSTDay = 31 - ((t + 4) % 7);
  int startDSTMonth = 3;

  //last sunday of october
  int endDSTDay = 31 - ((t + 1) % 7);
  int endDSTMonth = 10;

  // DST is valid as:
  if (((month >= startDSTMonth) && (day >= startDSTDay))
      || ((month <= endDSTMonth) && (day < endDSTDay))) {
    return 1;  // DST europe = utc +2 hour
  } else {
    return 0; // nonDST europe = utc +1 hour
  }
}

