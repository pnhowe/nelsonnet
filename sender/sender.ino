#include <SPI.h>
#include <RH_RF95.h>

#include "radio.h"

const char SENDER_NAME[] = "test01";

int16_t value1;
int16_t value2;
int16_t value3;
bool dirty1;
bool dirty2;
bool dirty3;

// must be named "values"
const value_entry values[4] = {
  {"test1", &value1, &dirty1},
  {"test2", &value2, &dirty2},
  {"test3", &value3, &dirty3},
  {"\0", 0, 0}
};

void setup()
{
  pinMode( LED, OUTPUT );
  digitalWrite( LED, HIGH );

  while( !Serial );
  Serial.begin( 9600 );
  delay( 1000 );

  Serial.println( "NelsonNet Sender" );
  Serial.println( SENDER_NAME );

  value1 = 5;
  value2 = 89;
  value3 = 0;
  dirty1 = 1;
  dirty2 = 1;
  dirty3 = 1;

  radio_setup( 10 );
  setupSender( SENDER_NAME, values );
  sync();

  Serial.println( "Setup Complete" );
  digitalWrite( LED, LOW );
}

void loop() 
{
  Serial.println( "... Loop ..." );
  for( int i = 0; i < 5000; i++ )
  {
    value1 = ( value1 + 1 ) % 30;
    value2 = ( value2 + 2 ) % 100;
    value3 = ( value3 + 3 ) % 200;
    dirty1 = 1;
    dirty2 = 1;
    if( i % 10 == 0 )
      dirty3 = 1;
    for( int j = 0; j < 100; j++)
    {
      senderProcess();
      delay( 100 );
    }
  }
}
