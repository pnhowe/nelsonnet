#include <SPI.h>
#include <RH_RF95.h>
#include <RTCZero.h>

#include "radio.h"

const value_entry *data_values;

int _offset;
int _sendCounter;
bool _doSend;
const char *_node_name;

const char SYNC_MSG[] = "-SYNC-";

char _sending_buff[METRIC_MSG_SIZE];

RTCZero rtc;
RH_RF95 rf95( RFM95_CS, RFM95_INT );

void radio_setup( int power )
{
  pinMode( RFM95_RST, OUTPUT );
  digitalWrite( RFM95_RST, HIGH );

  // manual reset
  digitalWrite( RFM95_RST, LOW );
  delay( 10 );
  digitalWrite( RFM95_RST, HIGH );
  delay( 10 );

  if( !rf95.init() )
  {
    Serial.println( "LoRa radio init failed" );
    while( 1 );
  }
  Serial.println( "LoRa radio init OK!" );

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if( !rf95.setFrequency( RF95_FREQ ) )
  {
    Serial.println( "setFrequency failed" );
    while( 1 );
  }
  Serial.print( "Set Freq to: " );
  Serial.println( RF95_FREQ );

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower( power, false );

  _doSend = false;
}

RH_RF95 *getRF95()
{
  return &rf95;
}

void sendValues()
{
  _doSend = false;
  char *cursor = _sending_buff;
  memset(_sending_buff, '\0', sizeof(_sending_buff));

  Serial.println( "--- Send Values ---" );

  _sendCounter++;
  Serial.print( "Send Counter: ");
  Serial.println(_sendCounter, DEC );
  cursor += snprintf( cursor, sizeof( _sending_buff ), "$\t%s:%i\t", _node_name, rf95.lastRssi() );
  for( int i = 0; i < 10; i++ )
  {  
    if( data_values[i].name[0] == '\0' )
      break;

    if( !(*data_values[i].dirty) )
      continue;

    cursor += snprintf( cursor, sizeof( _sending_buff ) - strlen( _sending_buff ) - 2, "%s:%u\t", data_values[i].name, *data_values[i].value );
    *data_values[i].dirty = 0;
  }
  *(cursor++) = '$';
  *(cursor++) = '\0';
  Serial.println(_sending_buff);
  Serial.println(strlen(_sending_buff), DEC);
  rf95.waitCAD();
  rf95.send( (uint8_t*) _sending_buff, strlen(_sending_buff) );
  rf95.waitPacketSent();
  Serial.println( "--- Values Sent ---" );
}

void _setDoSend()
{
  _doSend = true;
}

void setupSender( const char *name, const value_entry *values )
{
  uint8_t wrk;
  int name_len = strlen( name );
  for( int i = 0; i < name_len; i++ )
    wrk += (unsigned char) name[i]; // yes this will overflow, that is fine
  
  _node_name = name;
  _offset = wrk % SLICE_COUNT;
  Serial.print("Offset: ");
  Serial.println(_offset, DEC);

  data_values = values;

  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setAlarmSeconds( _offset );
  rtc.attachInterrupt( _setDoSend );
}

void sync()
{
  char buff[20];
  uint8_t len;
  rtc.disableAlarm();
  memset(buff, '\0', sizeof(buff));
  Serial.println( "___ waiting for sync ___" );
  while( 1 )
  {
    while( !rf95.available() )
      ;
    len = sizeof( buff );
    if( !rf95.recv( (uint8_t*) buff, &len ) )
      continue;

    Serial.print( "Len: " );
    Serial.println(len, DEC );
    Serial.print( "Got: " );
    Serial.println( (char*) buff );
    Serial.print( "RSSI: " );
    Serial.println( rf95.lastRssi(), DEC );

    if( ( len == strlen( SYNC_MSG ) ) && ( strncmp( buff, SYNC_MSG, len ) == 0 ) )
      break;
  }
  Serial.println( "___ Synced ___" );
  _sendCounter = 0;
  rtc.setTime(0, 0, 0);
  rtc.enableAlarm( rtc.MATCH_SS );
}

void sendSync()
{
  Serial.println( "___ send sync ___" );
  rf95.send( (uint8_t*) SYNC_MSG, strlen( SYNC_MSG ) );
  rf95.waitPacketSent();
}

void senderProcess()
{
  digitalWrite( LED, HIGH );
  if( _doSend )
    sendValues();

  if( _sendCounter >= 10 ) // >= 60 ) // every 60 min
    sync();
  
  digitalWrite( LED, LOW );
}
