#include <SPI.h>
#include <RH_RF95.h>
#include <RTCZero.h>

#include "radio.h"

unsigned int curOffset;

bool doSync = false;

RTCZero rtc2;

bool doPrint = false;

void updateOffset()
{
  curOffset += 5;
  curOffset %= 60;
  rtc2.setAlarmMinutes( curOffset );
  doSync = true;
}

void setup()
{
  pinMode( LED, OUTPUT );
  digitalWrite( LED, HIGH );

  while( !Serial );
  Serial.begin( 9600 );
  delay( 1000 );

  Serial.println( "NelsonNet Reciever" );

  radio_setup( 10 );

  curOffset = 0;
  rtc2.begin();
  rtc2.setTime(0, 0, 0);
  rtc2.setAlarmSeconds( 0 );
  rtc2.setAlarmMinutes( curOffset );
  rtc2.enableAlarm( rtc2.MATCH_MMSS );
  rtc2.attachInterrupt( updateOffset );

  doSync = false;

  Serial.println( "Setup Complete" );
  digitalWrite( LED, LOW );
}

void loop()
{
  // Should be a message for us now   
  uint8_t buff[ 64 ];
  uint8_t len;
  RH_RF95 *rf95 = getRF95();

  while( 1 )
  { 
    Serial.println( "... Loop ..." );
    while( !( rf95->available() || doSync ) )
      delay( 100 );

    if( doSync )
    {
      doSync = false;
      sendSync();
      continue;
    }

    if( rf95->available() )
    {
      Serial.println( "... Read Data..." );
      digitalWrite( LED, HIGH );
      memset( buff, '\0', sizeof( buff ) );
      len = sizeof( buff );  
      if( rf95->recv( buff, &len ) )
      {
        Serial.print( "!\t" );
        Serial.print( len, DEC );
        Serial.println( "\t!" );
        Serial.println( (char*)buff );
        Serial.print( "#\t" );
        Serial.print( rf95->lastRssi(), DEC );
        Serial.println( "\t#" );
        // add rssi as a metric after each batch of values are recieved
      }
      else
      {
        Serial.println("Receive failed");
      }
      digitalWrite(LED, LOW);
    }
  }
}
