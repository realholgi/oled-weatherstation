# oled-weatherstation
ESP8266 OLEDWeather Station

My weather station displays time, temperature, relative and absolute humidity and their absolute difference.

Hardware:

- Wemos D1 mini pro
- Arduino Mini Pro
- 2.42" OLED 128x64 OLED, 
- two SHT21 sensors
- 433Mhz transmitter and receiver

Features:

- starts in configuration mode its own WiFi portal on double-reset
- saves config data in SPIFFS
- displays data in local webpage found at http://weather.local and updates automatically
- transmits all data to UbiDots
- updates time from Internet
- firmware OTA updates
