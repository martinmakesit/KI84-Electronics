#include <Arduino.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>

const char* defaultSsid = "YOUR_WIFI_SSID";
const char* defaultPassword = "YOUR_PASSWORD";

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
bool lightState = false;    // Stores the current state of the lights
bool lightFlashing = false; // New variable to control flashing state
String lightControl = "";
bool motorState = false;    // Stores the current state of the motor
bool buttonState = HIGH;    // Stores the current state of the button
bool lastButtonState = HIGH;    // Stores the previous state of the button

AsyncWebServer server(80);  // Create AsyncWebServer instance on port 80

void handleRootPage(AsyncWebServerRequest *request) {
  String htmlPage = "<html><head><title>Ki-84 Electronics Tutorial</title></head><body>";

  // Section 1: Motor Control
  htmlPage += "<h2>Motor Control</h2>";
  htmlPage += "<button onclick=\"startMotor()\">Start Motor</button>";
  htmlPage += "<button onclick=\"stopMotor()\">Stop Motor</button>";

  // Section 2: Light Controls
  htmlPage += "<h2>Light Controls</h2>";
  htmlPage += "<form action=\"/lights\" method=\"POST\">";
  htmlPage += "<input type=\"radio\" name=\"lightControl\" value=\"off\"> Off";
  htmlPage += "<input type=\"radio\" name=\"lightControl\" value=\"on\"> On";
  htmlPage += "<input type=\"radio\" name=\"lightControl\" value=\"flashing\"> Flashing";
  htmlPage += "<br>";
  htmlPage += "<button type=\"submit\">Submit</button>";
  htmlPage += "</form>";

  htmlPage += "<script>";
  htmlPage += "function startMotor() {";
  htmlPage += "fetch('/motor?command=start');";
  htmlPage += "}";
  htmlPage += "function stopMotor() {";
  htmlPage += "fetch('/motor?command=stop');";
  htmlPage += "}";
  htmlPage += "</script>";

  htmlPage += "</body></html>";

  request->send(200, "text/html", htmlPage);
}

void handleMotorControl(AsyncWebServerRequest *request) {
  Serial.println("Got Motor Request");
  String command = request->getParam("command")->value();
  
  if (command == "start") {
    Serial.println("Got Motor Start");
    motorState = true;
    motorPower = 0; // Start at 0% power
    motorPreviousMillis = millis();
    mp3->begin(file, out);
  } else if (command == "stop") {
    motorState = false;
    analogWrite(motorPin, 0); // Turn off the motor
    mp3->stop();
  }

  request->send(200, "text/plain", "OK");
}

void handleLightControl(AsyncWebServerRequest *request) {
  Serial.println("Got Light Request");
  if (request->hasArg("lightControl")) {
  }
  if (request->method() == HTTP_POST) {
    if (request->hasArg("lightControl")) {
      lightControl = request->arg("lightControl");
      Serial.println(lightControl);
    }
  }

  request->send(200, "text/plain", "OK");
}

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

  // Connect to Wi-Fi
  WiFi.begin(defaultSsid, defaultPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize the web server
  server.on("/", HTTP_GET, handleRootPage);
  server.on("/motor", HTTP_GET, handleMotorControl);
  server.on("/lights", HTTP_POST, handleLightControl);

  // Start the server
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();  // Get the current time

  // Light flashing
  if (currentMillis - lightPreviousMillis >= lightInterval) {
    lightPreviousMillis = currentMillis;
    if (lightControl == "off"){
      lightState = false;
    }
    else if (lightControl == "on") {
      lightState = true;
     } 
     else if (lightControl == "flashing") {
      lightState = !lightState;
     }
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
          mp3->stop();
        } else {
          motorState = true;
          motorPower = 0; // Start at 0% power
          motorPreviousMillis = currentMillis;
          mp3->begin(file, out);
        }
      }
      lastButtonState = buttonState;
    }
  }

  // Motor control
  if (motorState) {
    unsigned long motorElapsedMillis = currentMillis - motorPreviousMillis;

    if (motorElapsedMillis <= rampUpStage1) {
      motorPower = map(motorElapsedMillis, 0, rampUpStage1, 0, 25);
    } else if (motorElapsedMillis <= rampUpStage2) {
      motorPower = map(motorElapsedMillis, rampUpStage1, rampUpStage2, 25, 50);
    } else if (motorElapsedMillis <= rampUpStage3) {
      motorPower = map(motorElapsedMillis, rampUpStage2, rampUpStage3, 50, 100);
    } else if (motorElapsedMillis <= rampUpStage4) {
      motorPower = map(motorElapsedMillis, rampUpStage3, rampUpStage4, 100, 255);
    } else if (motorElapsedMillis <= motorDuration) {
      motorPower = 255; // Maximum power after ramp-up stages
    } else {
      motorState = false;
      analogWrite(motorPin, 0); // Turn off the motor
      mp3->stop();
    }

    analogWrite(motorPin, motorPower);
  }

  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }
}
