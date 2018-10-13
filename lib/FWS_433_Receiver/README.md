# FWS 433

Use your Arduino to receive temperature and humidity data from FWS remote sensor.

## Download
https://github.com/realholgi/FWS433

## Info
### Hardware

There are several 433 MHz receiver available in many places. I used a cheap type and it works perfectly. You should use Arduino's interrupt enabled digital input pins (on Uno/Nano: D2 or D3) since this library built on top of that feature.

### Usage

Use at your own risk! Truly! There is no warranty!

In the examples directory you can find a simple way of usage.

## Protocol

You can find a detailed description of the protocol here: https://forum.pilight.org/showthread.php?tid=3080&page=2#pid21959

### Inspiration

Thanks for the reverse engineering the sensor's messages to pilight's dev team. The processing code is built on top of their excellent source - tailored for my special needs and limited environment.
