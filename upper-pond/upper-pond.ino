#include <HCSR04.h>
#include <SPI.h>
#include "radio.h"

// requires the RadioHead, RTCZero, "HCSR04 ultrasonic sensor" by gamegine libraries


HCSR04 hc(12,11);

#define SAMPLE_DELAY 100

const int batteryPin = A3;

extern const char SENDER_NAME[];
const char SENDER_NAME[] = "upper_pond";

int16_t value_level;
int16_t value_battery;
bool dirty_level;
bool dirty_battery;

extern const value_entry data_values[];
const value_entry data_values[] = {
  { "level", &value_level, &dirty_level },
  { "batt", &value_battery, &dirty_battery },
  { "\0", 0, 0 }
};

bool doPrint = false;

void setup() {
  pinMode( LED, OUTPUT );
  digitalWrite( LED, HIGH );

  uint16_t counter = 200;
  while( !Serial && counter )
  {
    counter--;
    delay( 10 );
  }
  if( counter )
  {
    Serial.begin( 9600 );
    doPrint = true;
  }
  delay( 1000 );
  
  Println( "NelsonNet Sender" );
  Println( SENDER_NAME );

  radio_setup( 10 );
  setupSender();
  sync();

  value_level = hc.dist();
  value_battery = 0;
  dirty_level = 0;
  dirty_battery = 0;

  Println( "Setup Complete" );
  digitalWrite( LED, LOW );
}

void loop() {
  Println( "... Loop ..." );
  value_level = ( hc.dist() * 50 + value_level * 50 ) / 100;
  dirty_level = 1;

  value_battery = analogRead( batteryPin );
  dirty_battery = 1;

  for( int j = 0; j < SAMPLE_DELAY; j++)
  {
    senderProcess();
    delay( 100 );
  }
}

