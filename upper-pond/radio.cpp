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
    SerialUSB.println( "LoRa radio init failed" );
    while( 1 );
  }
  SerialUSB.println( "LoRa radio init OK!" );

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if( !rf95.setFrequency( RF95_FREQ ) )
  {
    SerialUSB.println( "setFrequency failed" );
    while( 1 );
  }
  SerialUSB.print( "Set Freq to: " );
  SerialUSB.println( RF95_FREQ );

  rf95.setTxPower( power, false );

  _doSend = false;
}

RH_RF95 *getRF95()
{
  return &rf95;
}

void sendValues()
{
  // _doSend = false;
  char *cursor = _sending_buff;
  memset(_sending_buff, '\0', sizeof(_sending_buff));

  SerialUSB.println( "--- Send Values ---" );

  _sendCounter++;
  SerialUSB.print( "Send Counter: ");
  SerialUSB.println(_sendCounter, DEC );
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
  SerialUSB.println(_sending_buff);
  SerialUSB.println(strlen(_sending_buff), DEC);
  rf95.waitCAD();
  rf95.send( (uint8_t*) _sending_buff, strlen(_sending_buff) );

  delay(10);
  rf95.waitPacketSent();
  SerialUSB.println( "--- Values Sent ---" );
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
  SerialUSB.print("Offset: ");
  SerialUSB.println(_offset, DEC);

  data_values = values;

  rtc.begin();
  rtc.setTime(0, 0, 0);
  rtc.setAlarmSeconds( _offset );
  rtc.attachInterrupt( _setDoSend );
  _doSend = true;
}

void sync()
{
  char buff[20];
  uint8_t len;
  rtc.disableAlarm();
  memset(buff, '\0', sizeof(buff));
  SerialUSB.println( "___ waiting for sync ___" );
  while( 1 )
  {
    while( !rf95.available() )
      ;
    len = sizeof( buff );
    if( !rf95.recv( (uint8_t*) buff, &len ) )
      continue;

    SerialUSB.print( "Len: " );
    SerialUSB.println(len, DEC );
    SerialUSB.print( "Got: " );
    SerialUSB.println( (char*) buff );
    SerialUSB.print( "RSSI: " );
    SerialUSB.println( rf95.lastRssi(), DEC );

    if( ( len == strlen( SYNC_MSG ) ) && ( strncmp( buff, SYNC_MSG, len ) == 0 ) )
      break;
  }
  SerialUSB.println( "___ Synced ___" );
  _sendCounter = 0;
  rtc.setTime(0, 0, 0);
  rtc.enableAlarm( rtc.MATCH_SS );
}

void sendSync()
{
  SerialUSB.println( "___ send sync ___" );
  rf95.send( (uint8_t*) SYNC_MSG, strlen( SYNC_MSG ) );
  rf95.waitPacketSent();
}

void senderProcess()
{
  digitalWrite( LED, HIGH );
  if( _doSend )
    sendValues();

  // if( _sendCounter >= 10 ) // >= 60 ) // every 60 min
  //   sync();
  
  digitalWrite( LED, LOW );
}

