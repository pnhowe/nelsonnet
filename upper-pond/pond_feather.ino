#include <HCSR04.h>
#include <SPI.h>
#include <RH_RF95.h>
#include "radio.h"

#define LED 13

#define SENDER_NAME "upper_pond"
#define SAMPLE_DELAY 10000

HCSR04 hc(12,11);

const int batteryPin = A3;

int16_t value_level;
int16_t value_battery;
bool dirty_level;
bool dirty_battery;

const value_entry values[3] = {
  { "level", &value_level, &dirty_level },
  { "batt", &value_battery, &dirty_battery },
  { "\0", 0, 0 }
};

void setup() {
  pinMode(LED, OUTPUT);

  SerialUSB.begin(115200);

  digitalWrite(LED, true);
  
  // wait for the serial port to open
  delay(4000);

  SerialUSB.println("Hello, Pond Sensor!");
  SerialUSB.println("");

  radio_setup(18);
  setupSender(SENDER_NAME, values);

  digitalWrite(LED, false);
}

void loop() {
  value_level = hc.dist();
  dirty_level = 1;

  value_battery = analogRead(batteryPin);
  dirty_battery = 1;

  senderProcess();

  delay(SAMPLE_DELAY);
}

