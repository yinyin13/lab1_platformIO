#include <Arduino.h>
#include "Adafruit_VEML7700.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

// Define the LED pin
#define LED_PIN 20 // Replace with the actual GPIO pin number

Adafruit_BME280 bme; // I2C
Adafruit_VEML7700 veml = Adafruit_VEML7700();
Adafruit_SSD1306 display(128, 32, &Wire);

const int filterSize = 10; // Adjust the filter size as needed
float temperatureHistory[filterSize];
float luxHistory[filterSize];

// Function prototypes
void updateHistory(float history[], float newValue);
float calculateAverage(float history[]);

void setup() {
  Serial.begin(115200);
  Serial.println("VEML7700 demo");

  while (!Serial);    // time to get serial running
  Serial.println(F("BME280 test"));

  unsigned status;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // default settings
  status = bme.begin(0x76);
  if (!status) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("BME280 Sensor not found!"));
    display.display();
    while (1) delay(10);
  }

  if (veml.begin()) {
    Serial.println("Found a VEML7700 sensor");
  } else {
    Serial.println("No sensor found ... check your wiring?");
    while (1);
  }

  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;); // Don't proceed, loop forever
  // }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);

  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_100MS);

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Initialize history arrays
  for (int i = 0; i < filterSize; i++) {
    temperatureHistory[i] = bme.readTemperature();
    luxHistory[i] = veml.readLux();
  }
}

void loop() {
  // Read temperature
  float temperature = bme.readTemperature();
  
  // Read lux
  float lux = veml.readLux();

  // Update history arrays
  updateHistory(temperatureHistory, temperature);
  updateHistory(luxHistory, lux);

  // Calculate filtered temperature
  float filteredTemperature = calculateAverage(temperatureHistory);

  // Calculate filtered lux
  float filteredLux = calculateAverage(luxHistory);

  // Send data over serial
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(filteredTemperature);
  Serial.print(",");
  Serial.print(lux);
  Serial.print(",");
  Serial.println(filteredLux);

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(F("Temp: "));
  display.print(filteredTemperature);
  display.write(char(223));
  display.println("C");

  display.setCursor(0, 10);
  display.print(F("Lux: "));
  display.println(filteredLux, 2);

  if (filteredLux < 50 || filteredTemperature < 10) {
    display.setCursor(0, 20);
    display.println(F("LED lit"));
    digitalWrite(LED_PIN, HIGH);
  } else {
    display.setCursor(0, 20);
    display.println(F("LED off"));
    digitalWrite(LED_PIN, LOW);
  }

  display.display();
  delay(50);
}

void updateHistory(float history[], float newValue) {
  // Shift elements in the history array
  for (int i = 0; i < filterSize - 1; i++) {
    history[i] = history[i + 1];
  }

  // Add the new value to the history array
  history[filterSize - 1] = newValue;
}

float calculateAverage(float history[]) {
  float sum = 0;

  // Calculate the sum of elements in the history array
  for (int i = 0; i < filterSize; i++) {
    sum += history[i];
  }

  // Calculate and return the average
  return sum / filterSize;
}