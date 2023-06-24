#include <Arduino.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include <EEPROM.h>

#include <ESP8266WiFi.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


String ssid = "YOUR_WIFI_SSID";
String password = "YOUR_PASSWORD";

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

// Define the pins for the leds and the motor
#define LED1 12
#define LED2 14
#define MOTOR 4
#define BUTTON 5

// Define the intervals for changing motor speed in milliseconds
const int SPEED_CHANGE_INTERVAL_1 = 4000;  // 4 seconds
const int SPEED_CHANGE_INTERVAL_2 = 6500;
const int SPEED_CHANGE_INTERVAL_3 = 7500;   // 2 seconds
const int SPEED_CHANGE_INTERVAL_4 = 30000;  // 2 seconds

// Declare variables to store the current and previous millis values
unsigned long currentMillisMotor;
unsigned long currentMillisLED;
unsigned long previousMillisMotor;
unsigned long previousMillisLED;

// Declare a variable to store the state of the leds
bool ledState;
bool reconnectToWifi = false;
int ledMode = 1;

// Declare a variable to store the PWM value for the motor
int motorSpeed = 0;
const int motorInitialValue = 70;
const int motorFirstValue = 110;
const int motorSecondValue = 140;
const int motorThirdValue = 180;

// variable for reading the pushbutton status
bool buttonState = HIGH;

// BOOL to check if the motor sequence is running
bool motorSequenceState = false;

AsyncWebServer server(80);


void handleRoot(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html");
}

void handleStopMotor(AsyncWebServerRequest *request) {
  // Your stop motor code goes here
  motorSpeed = 0;
  analogWrite(MOTOR, motorSpeed);
  motorSequenceState = false;
  Serial.println(F("Motor Stopped"));
  mp3->stop();
  request->redirect("/");
}


void handleStartMotor(AsyncWebServerRequest *request) {
  if (motorSequenceState == false) {
    previousMillisMotor = currentMillisMotor;
    motorSequenceState = true;
    // turn on motorsequence:
    delete file;
    delete out;
    //delete mp3;
    //Serial.println("delete mp3");
    file = new AudioFileSourceSPIFFS("/ZeroStartupSmall.mp3");
    out = new AudioOutputI2S();
    mp3 = new AudioGeneratorMP3();
    //out->SetGain(4);
    mp3->begin(file, out);
    motorSpeed = motorInitialValue;
    analogWrite(MOTOR, motorSpeed);
    Serial.print(F("Motor speed set to: "));
    Serial.println(motorSpeed);
  }
  request->redirect("/");
}

void handleSetLedMode(AsyncWebServerRequest *request) {
  if (request->hasArg("mode")) {
    ledMode = request->arg("mode").toInt();
  }
  request->send(302, "text/plain", "");  // Send a response to redirect the user
  request->redirect("/");                // Redirect the user to the root page
}

void handleWifiConfig(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/wificonfig.html", "text/html");
  Serial.println("sent wificonfig");
}

void handleSetWifiConfig(AsyncWebServerRequest *request) {
  if (request->hasArg("ssid") && request->hasArg("password")) {
    ssid = request->arg("ssid");
    password = request->arg("password");
    reconnectToWifi = true;
    // Disconnect from the current network and try to connect to the new network

  }
}

void clearEEPROM() {
  // Create a buffer of 64 bytes filled with zeros
  byte buffer[96] = {0};

  // Write the buffer to the first 64 bytes of the EEPROM
  EEPROM.put(0, buffer);

  // Commit the changes to the EEPROM
  EEPROM.commit();
}

void saveCredentials(String ssidToSave, String passwordtoSave) {
  EEPROM.begin(512);
  char ssidChar[32];
  ssidToSave.toCharArray(ssidChar, sizeof(ssidChar));
  char passwordChar[32];
  passwordtoSave.toCharArray(passwordChar, sizeof(passwordChar));
  EEPROM.put(0, ssidChar);
  EEPROM.put(32, passwordChar);
  EEPROM.commit();
  EEPROM.end();
}

void loadCredentials() {
  EEPROM.begin(512);
  char stored_ssid[32];
  char stored_password[64];
  EEPROM.get(0, stored_ssid);
  EEPROM.get(32, stored_password);
  EEPROM.end();

  // Check if the stored SSID is empty
  if (strlen(stored_ssid) == 0) {
    Serial.println(F("No credentials found in EEPROM. Using default credentials."));
    // Save the default credentials to EEPROM
    saveCredentials(ssid, password);
  } else {
    Serial.println(String(stored_ssid));
    Serial.println(String(stored_password));
    ssid = String(stored_ssid);
    password = String(stored_password);
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  delay(2000);
  // Load the saved WiFi credentials
  loadCredentials();
  WiFi.hostname("Ki-84 Model - ESP8266");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for 1 minute to check if the device connects to the Wi-Fi network
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(1000);
    Serial.print(".");
  }

  // Check if the device has not connected within the time limit and create a Wi-Fi access point
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    Serial.println("Connection Failed! Creating Access Point...");

    // Set a static IP address for the access point
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // Start the access point with the desired SSID and password
    WiFi.softAP("Ki84", "stupidpassword");


    Serial.print("Access Point started. IP address: ");
    Serial.println(apIP);
    clearEEPROM();
  } else {
    // Connected to Wi-Fi successfully
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


  }

  SPIFFS.begin();
  file = new AudioFileSourceSPIFFS("/ZeroStartupSmall.mp3");
  out = new AudioOutputI2S();
  mp3 = new AudioGeneratorMP3();
  //out->SetGain(4);

  // Set the pins as outputs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  // Initialize the leds to LOW
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // Initialize the current and previous millis values to zero
  currentMillisMotor = 0;
  currentMillisLED = 0;
  previousMillisMotor = 0;
  previousMillisLED = 0;

  // Initialize the led state to false
  ledState = false;

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleRoot(request);
  });
  server.on("/startMotor", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleStartMotor(request);
  });
  server.on("/setLedMode", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleSetLedMode(request);
  });
  server.on("/stopMotor", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleStopMotor(request);
  });
  server.on("/wifiConfig", HTTP_GET, [](AsyncWebServerRequest * request) {
    handleWifiConfig(request);
  });
  server.on("/setWifiConfig", HTTP_POST, [](AsyncWebServerRequest * request) {
    handleSetWifiConfig(request);
  });
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println(F("HTTP server started"));
}

void loop() {

  buttonState = digitalRead(BUTTON);
  // Get the current millis value
  currentMillisMotor = millis();
  currentMillisLED = millis();

  if (buttonState == LOW && motorSequenceState == false) {
    // handleStartMotor();
  }

  if (motorSequenceState == true) {
    // Check if 4 seconds have passed since the last speed change
    if (currentMillisMotor - previousMillisMotor >= SPEED_CHANGE_INTERVAL_1 && motorSpeed < motorFirstValue) {

      // Set the motor speed to the first value
      motorSpeed = motorFirstValue;

      // Set the motor speed using PWM
      analogWrite(MOTOR, motorSpeed);

      // Print debug message to serial monitor
      //Serial.print("Motor speed set to: ");
      //Serial.println(motorSpeed);
    }

    // Check if 2 seconds have passed since the last speed change
    if (currentMillisMotor - previousMillisMotor >= SPEED_CHANGE_INTERVAL_2 && motorSpeed < motorSecondValue) {


      // Set the motor speed to the second value
      motorSpeed = motorSecondValue;

      // Set the motor speed using PWM
      analogWrite(MOTOR, motorSpeed);

      // Print debug message to serial monitor
      //Serial.print("Motor speed set to: ");
      //Serial.println(motorSpeed);
    }

    // Check if 6 seconds have passed since the last speed change
    if (currentMillisMotor - previousMillisMotor >= (SPEED_CHANGE_INTERVAL_3) && motorSpeed < motorThirdValue) {

      // Set the motor speed to 100%
      motorSpeed = motorThirdValue;

      // Set the motor speed using PWM
      analogWrite(MOTOR, motorSpeed);

      // Print debug message to serial monitor
      //Serial.print("Motor speed set to: ");
      //Serial.println(motorSpeed);
    }
    if (currentMillisMotor - previousMillisMotor >= (SPEED_CHANGE_INTERVAL_4)) {
      mp3->stop();
      motorSpeed = 0;
      analogWrite(MOTOR, motorSpeed);
      motorSequenceState = false;

      //Serial.println("Sequence Ended");
    }
  }
  // Toggle the led state from false to true or vice versa every second
  if (currentMillisLED - previousMillisLED >= 1000) {
    // Update the previous millis value to the current one
    previousMillisLED = currentMillisLED;

    switch (ledMode) {
      case 0:
        digitalWrite(LED1, !digitalRead(LED1));
        digitalWrite(LED2, digitalRead(LED1));
        //Serial.println("Together ");
        break;
      case 1:
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        //Serial.println("Solid");
        break;
      case 2:
        digitalWrite(LED1, !digitalRead(LED1));
        digitalWrite(LED2, !digitalRead(LED1));
        //Serial.println("Alternating");
        break;
      case 3:
        // Turn both LEDs off
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        break;
    }
  }


  if (reconnectToWifi) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // Wait for the connection to be established
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(1000);
      Serial.print(".");
    }

    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      // Connection succeeded
      Serial.println("Connected to the new Wi-Fi network");
      saveCredentials(ssid.c_str(), password.c_str()); // Save the new credentials
      Serial.println(ssid.c_str());
      Serial.println(password.c_str());
      loadCredentials();
      Serial.println(ssid);
      Serial.println(password);
      reconnectToWifi = false;
      WiFi.softAP("");
      //request->send(200, "text/plain", "Successfully connected to the new Wi-Fi network!");
    } else {
      // Connection failed
      Serial.println("Failed to connect to the new Wi-Fi network");
      reconnectToWifi = false;
      WiFi.disconnect(true);
      WiFi.mode(WIFI_AP);
      Serial.println("Connection Failed! Creating Access Point...");

      // Set a static IP address for the access point
      IPAddress apIP(192, 168, 4, 1);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

      // Start the access point with the desired SSID and password
      WiFi.softAP("Ki84", "stupidpassword");
      clearEEPROM();
      //request->send(200, "text/plain", "Failed to connect to the new Wi-Fi network. Please try again.");
    }
  } else {
    //request->send(400, "text/plain", "Invalid request. Both SSID and password must be provided.");
  }
  //server.handleClient();
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }
}
