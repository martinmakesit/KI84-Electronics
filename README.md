# KI84-Electronics
This contains the code and schematics for my KI-84 electronics.

As a note, I'm a bad coder and this project is right on the border with the memory of the esp8266.  I'm sure there are a ton of better ways to do everything in here.  But hey, it works I guess?

## How to use:

### This requires the following:

ESP8266 Boards in Arduino IDE: https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/


### Libraries to install (Sketch->Include Library->Manage Libraries)

ESP8266Audio: https://github.com/earlephilhower/ESP8266Audio

ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP

AsyncElegantOTA: https://github.com/ayushsharma82/AsyncElegantOTA



## Features

Web Control of Engine and Lights

Over the Air Updates

Backup wifi hotspot config

## Parts List:
ESP8266 Mini

MAX98357 I2S Audio Amplifier Module

3-5v DC Motor

Resistors: 220, 10k

Mosfet: P30N06LE or some other N-channel logic level mosfet

Capacitor (for motor): .1uf

Speakers

LEDs

Schottky Diode

## Schematic
NOT TESTED, this was done after the fact from memory.
![Schematic](https://github.com/martinmakesit/KI84-Electronics/blob/main/ki84_bb.png "Schematic")
