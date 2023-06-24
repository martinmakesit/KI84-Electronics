# KI84-Electronics
This contains the code and schematics for my KI-84 electronics.

As a note, I'm a bad coder and this project is right on the border with the memory of the esp8266.  I'm sure there are a ton of better ways to do everything in here.  But hey, it works I guess?

## How to use:

### This requires the following:

ESP8266 Boards in Arduino IDE: https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/


### Libraries to install (Sketch->Include Library->Manage Libraries)

ESP8266Audio: https://github.com/earlephilhower/ESP8266Audio

ESPAsyncTCP: https://github.com/me-no-dev/ESPAsyncTCP

ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer

AsyncElegantOTA: https://github.com/ayushsharma82/AsyncElegantOTA


## Tutorials Used from https://randomnerdtutorials.com/

[SPIFFS filesystem and sketch data upload](https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/)

[ESPAsyncWebServer](https://randomnerdtutorials.com/esp8266-nodemcu-wi-fi-manager-asyncwebserver/)

[ESP8266 Pinouts](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)

## Features

Webpage Control of Motor and Lights via WiFi

Over the Air Updates

Backup wifi hotspot config

## Parts List:
ESP8266 Dev Board

ESP8266 Mini

MAX98357 I2S Audio Amplifier Module

3-5v DC Motor

Resistors: 100, 10k

Mosfet: IRLB8721 or some other N-channel logic level mosfet

Capacitor (for motor): .1uf

Speakers

LEDs

Schottky Diode

5V power adapter

### Amazon Affiliate Links to parts used:

ESP8266 Dev Board: https://amzn.to/43W0EDe

ESP8266 Mini: https://amzn.to/43CtpF0

Arduino Uno Starter kit (for breadboard and various resistors): https://amzn.to/43AHGlw

Motor: https://amzn.to/3qFRUmj

LED: https://amzn.to/42WdIrd

MAX98357 I2S Audio Amplifier: https://amzn.to/3X4GN29

Speakers : https://amzn.to/3X5mt0A

Resistors: https://amzn.to/3MVvMLW

Mosfets:  https://amzn.to/3P58AO3

Soldiering Iron: https://amzn.to/3oZ3o3Q

Helping Hands: https://amzn.to/3oS1zWp

Power Adapter: https://amzn.to/46dssVo



## Schematic
NOT TESTED, this was done after the fact from memory.
![Schematic](https://github.com/martinmakesit/KI84-Electronics/blob/main/ki84_bb.png "Schematic")
