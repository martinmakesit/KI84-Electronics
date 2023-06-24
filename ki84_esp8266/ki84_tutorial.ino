#include <Arduino.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

const int lightPin1 = 12;   // Pin number for the first light
const int lightPin2 = 14;   // Pin number for the second light
const int motorPin = 4;     // Pin number for the motor
const int buttonPin = 5;    // Pin number for the button

unsigned long lightPreviousMillis = 0;    // Stores the previously recorded time for the lights
unsigned long motorPreviousMillis = 0;    // Stores the previously recorded time for the motor
unsigned long buttonPreviousMillis = 0;   // Stores the previously recorded time for the button

const unsigned long lightInterval = 1000; // Time interval (in milliseconds) for flashing the lights
const unsigned long rampUpStage1 = 2000;  // Duration (in milliseconds) for ramp-up stage 1 (10% power)
const unsigned long rampUpStage2 = 5000;  // Duration (in milliseconds) for ramp-up stage 2 (25% power)
const unsigned long rampUpStage3 = 3000;  // Duration (in milliseconds) for ramp-up stage 3 (50% power)
const unsigned long rampUpStage4 = 0;     // Duration (in milliseconds) for ramp-up stage 4 (100% power)
const unsigned long motorDuration = 30000; // Duration (in milliseconds) for motor to run before turning off
const int pwmFrequency = 1000;            // Frequency of the PWM signal (in Hz)

int motorPower = 0;   // Stores the current motor power level (0-255)
boolean lightState = false;    // Stores the current state of the lights
boolean motorState = false;    // Stores the current state of the motor
boolean buttonState = HIGH;    // Stores the current state of the button
boolean lastButtonState = HIGH;    // Stores the previous state of the button

void setup() {
  Serial.begin(115200);

  SPIFFS.begin();
  file = new AudioFileSourceSPIFFS("/ZeroStartupSmall.mp3");
  out = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();

  pinMode(lightPin1, OUTPUT);   // Set the first light pin as output
  pinMode(lightPin2, OUTPUT);   // Set the second light pin as output
  pinMode(motorPin, OUTPUT);    // Set the motor pin as output
  pinMode(buttonPin, INPUT_PULLUP);  // Set the button pin as input with internal pull-up resistor

  analogWriteFreq(pwmFrequency);   // Set the PWM frequency
  analogWriteRange(255);
  Serial.println("setup good");
}

void loop() {
  unsigned long currentMillis = millis();  // Get the current time

  // Light flashing
  if (currentMillis - lightPreviousMillis >= lightInterval) {
    lightPreviousMillis = currentMillis;

    lightState = !lightState;
    digitalWrite(lightPin1, lightState);
    digitalWrite(lightPin2, lightState);
  }

  // Button handling
  if (currentMillis - buttonPreviousMillis >= 50) {
    buttonPreviousMillis = currentMillis;
    buttonState = digitalRead(buttonPin);

    if (buttonState != lastButtonState) {
      if (buttonState == LOW) {
        if (motorState) {
          motorState = false;
          analogWrite(motorPin, 0); // Turn off the motor
        } else {
          Serial.println("Button Pushed");
          motorState = true;
          motorPower = 0; // Start at 0% power
          motorPreviousMillis = currentMillis;
          mp3->begin(file, out);
          Serial.println("mp3 play");
        }
      }
      lastButtonState = buttonState;
    }
  }

  // Motor control
  if (motorState) {
    unsigned long elapsedTime = currentMillis - motorPreviousMillis;

    if (elapsedTime < rampUpStage1) {
      motorPower = 255 * 0.1;  // 10% power
    } else if (elapsedTime < rampUpStage1 + rampUpStage2) {
      motorPower = 255 * 0.25; // 25% power
    } else if (elapsedTime < rampUpStage1 + rampUpStage2 + rampUpStage3) {
      motorPower = 255 * 0.5;  // 50% power
    } else {
      motorPower = 255;        // 100% power
    }

    analogWrite(motorPin, motorPower);

    // Check if motor duration exceeded 30 seconds
    if (currentMillis - motorPreviousMillis >= motorDuration) {
      motorState = false;
      analogWrite(motorPin, 0); // Turn off the motor
      mp3->stop();
    }
  } else {
    analogWrite(motorPin, 0); // Motor is off
  }

  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  } 
}
