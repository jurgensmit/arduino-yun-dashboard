/*
 * Arduino Yun Dashboard - Provides an interface via the serial port to the connnected sensors and leds
 *
 * Circuit: 
 *
 * - An HC-SR04 ultrasonic sensor connected to pin 13 (trigger) and pin 12 (echo) to measure the distance
 * - A potmeter connected to anlog port A0
 * - A temperature sensor connected to analog port A1
 * - A photoresistor connected to analog port A2
 * - A green led connected to pin 10
 * - A red led connected to pin 11
 * - A yellow led connected to pin 9
 */

#include <NewPing.h>
#include <Wire.h>
#include <math.h>
#include "rgb_lcd.h"

#define TRIGGER_PIN         13    // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN            12    // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE        42    // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define RED_LED_PIN         11    // Arduino pin tied to the anode of the red led
#define GREEN_LED_PIN       10    // Arduino pin tied to the anode of the green led
#define YELLOW_LED_PIN      9     // Arduino pin tied to the anode of the yellow led
#define TEMPERATURE_PIN     1     // Arduino (analog) pin tied to the temperature sensor
#define PHOTORESISTOR_PIN   2     // Arduino (analog) pin tied to the photoresistor
#define POTMETER_PIN        0     // Arduino (analog) pin tied to the potmeter
#define DISTANCE_TOKEN      "D:"  // Token to prepend to the output when outputing the distance to the serial port
#define TEMPERATURE_TOKEN   "T:"  // Token to prepend to the output when outputing the temperature to the serial port
#define ANGLE_TOKEN         "A:"  // Token to prepend to the output when outputing the angle to the serial port
#define PHOTORESISTOR_TOKEN "P:"  // Token to prepend to the output when outputing the light to the serial port
#define LEDSTATUS_TOKEN     "L:"  // Token to prepend to the output when outputing the led status to the serial port

NewPing sonar(TRIGGER_PIN, ECHO_PIN);  
rgb_lcd lcd;

void setup() {
  lcd.begin(16, 2);  // Set up the LCD's number of columns and rows:

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  
  lcd.clear();
  lcd.print("Dashboard");
  
  Serial1.begin(115200);
  
  turnOffLeds();
}

void turnOffLeds() {
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
}

/*
 * Send a single value via the serial port
 */
void sendValue(String token, unsigned int value) {
  Serial1.print(token);
  Serial1.println(value);
}

/*
 * Measure and return the distance
 */
unsigned int getDistance() {
  delay(50);  // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int distance = sonar.ping_cm(); // Send ping, get distance in centimeters.
  return distance;
}

/*
 * Print a message to the 2nd line of the LCD display
 */
void printMessage(const String& message) {
  lcd.setCursor(0, 1);
  lcd.print(message + String("                "));
}

/*
 * Serial Communication 
 */
byte commandBuffer[8];
int bufferPointer = 0;

/*
 * Process messages which arrive via the serial port
 */
void handleIncomingMessages() {
  if (Serial1.available() > 0) {
    if(bufferPointer < sizeof(commandBuffer))
    {
      byte data = Serial1.read();
      if(data == '@') {
        handleMessage();
        bufferPointer = 0;
      }
      else {
        if(data != '\r' && data != '\n') {
          commandBuffer[bufferPointer++] = data;
        }
      }
    }
  }
}

/*
 * Process a single message that has been received
 */
void handleMessage() {
  if(bufferPointer >= 2 && commandBuffer[1] == ':') {
    switch(commandBuffer[0]) {
      case 'L': 
        toggleLed(commandBuffer[2] - '0'); 
        break;
      case 'X':
        sendLatestValues();
        break;
    }
  }
}

/*
 * Toggle one off the leds, 0 = Yellow, 1 = Green, 2 = Red
 */
void toggleLed(byte ledIndex) {
  int ledPin = YELLOW_LED_PIN + ledIndex;
  digitalWrite(ledPin, !digitalRead(ledPin));
  sendAllLedsStatus();
}

// Initialize the measured sensor values to impossible values so they will be initialized
// by the first real measurement
unsigned int latestTemperature = -1;
unsigned int latestAngle = -1;
unsigned int latestDistance = -1;
unsigned int latestLight = -1;

/*
 * Measure and return the current temperature
 */
unsigned int getTemperature() {
  int a = analogRead(TEMPERATURE_PIN);
  float resistance = (float)(1023 - a) * 10000 / a; //get the resistance of the sensor
  int B = 3975; 
  float temperature = round(1 / (log(resistance/10000) / B + 1 / 298.15) - 273.15); //convert to temperature via datasheet
  return (unsigned int) temperature;
}

/*
 * Measure and return the value of the potmeter
 */
unsigned int getAngle() {
  return analogRead(POTMETER_PIN);
}

/*
 * Measure and return the value of the photoresistor
 */
unsigned int getLight() {
  return analogRead(PHOTORESISTOR_PIN);
}

/*
 * Send the latest measured distance via the serial port
 */
void sendLatestDistance() {
  sendValue(DISTANCE_TOKEN, latestDistance);
}

/*
 * Send the latest measured temperature via the serial port
 */
void sendLatestTemperature() {
  sendValue(TEMPERATURE_TOKEN, latestTemperature);
}

/*
 * Send the latest measured angle via the serial port
 */
void sendLatestAngle() {
  sendValue(ANGLE_TOKEN, latestAngle);
}

/*
 * Send the latest measured light via the serial port
 */
void sendLatestLight() {
  sendValue(PHOTORESISTOR_TOKEN, latestLight);
}

/*
 * Send the current status of the given led via the serial port
 */
void sendLedStatus(int ledIndex, int status) {
  Serial1.print(LEDSTATUS_TOKEN);
  Serial1.print(String(ledIndex));
  Serial1.println(status ? "1" : "0");
}

/*
 * Send the current status of the yellow led via the serial port
 */
void sendYellowLedStatus() {
  sendLedStatus(0, digitalRead(YELLOW_LED_PIN));
}

/*
 * Send the current status of the green led via the serial port
 */
void sendGreenLedStatus() {
  sendLedStatus(1, digitalRead(GREEN_LED_PIN));
}

/*
 * Send the current status of the red led via the serial port
 */
void sendRedLedStatus() {
  sendLedStatus(2, digitalRead(RED_LED_PIN));
}

/*
 * Send the status of all leds via the serial port
 */
void sendAllLedsStatus() {
  sendYellowLedStatus();
  sendGreenLedStatus();
  sendRedLedStatus();
}

/*
 * Send the current value of all sensots and status of all leds via the serial port
 */
void sendLatestValues() {
  sendLatestDistance();
  sendLatestTemperature();
  sendLatestAngle();
  sendLatestLight();
  sendAllLedsStatus();
}

void loop() {
  // Get the current sensor values
  unsigned int distance = getDistance();
  unsigned int temperature = getTemperature();
  unsigned int angle = getAngle();
  unsigned int light = getLight();
  
  // Send any changes to the serial port
  if(distance != latestDistance) {
    latestDistance = distance;
    sendLatestDistance();
  }
  
  if(temperature != latestTemperature) {
    latestTemperature = temperature;
    sendLatestTemperature();
  }
  
  if(angle != latestAngle) {
    latestAngle = angle;
    sendLatestAngle();
  }
  
  if(light != latestLight) {
    latestLight = light;
    sendLatestLight();
  }

  // Wait a little bit to not overload the serial port with updates    
  // But still handle incoming requests fast
  for(int i = 0; i < 50; i ++) {
    // Process any incoming requests from the serial port
    handleIncomingMessages();
    delay(5);
  }
}
