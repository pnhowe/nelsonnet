#include "radio.h"

// requires the RaduiHead and RTCZero libraries

// Modify name, max length of 10 (not including the null)
extern const char SENDER_NAME[];
const char SENDER_NAME[] = "test01";

// value of Values to send,myst be a int16_t
int16_t value1;
int16_t value2;
int16_t value3;

// if dirty = 1, then the value will be sent, and dirty will be reset back to 0
bool dirty1;
bool dirty2;
bool dirty3;

// must be named "data_values"
// First field is the name, max length of 10 (not including the null)
// max of 10 metrics will be sent
extern const value_entry data_values[];
const value_entry data_values[] = {
  {"test1", &value1, &dirty1},
  {"test2", &value2, &dirty2},
  {"test3", &value3, &dirty3},
  {"\0", 0, 0}
};

bool doPrint = false;

void setup()
{
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

  value1 = 5;
  value2 = 89;
  value3 = 0;
  dirty1 = 1;
  dirty2 = 1;
  dirty3 = 1;

  radio_setup( 10 );
  setupSender();
  sync();

  Println( "Setup Complete" );
  digitalWrite( LED, LOW );
}

void loop() 
{
  Println( "... Loop ..." );
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
