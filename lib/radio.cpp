#include <SPI.h>
#include <RH_RF95.h>
#include <RTCZero.h>

#include "radio.h"

extern bool doPrint;
extern const char SENDER_NAME[];
extern const value_entry data_values[];

int _offset;
uint16_t _sendCounter = 0;
uint16_t _syncCounter = 0;
bool _doSend = false;
bool _doSync = true;

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
    Println( "LoRa radio init failed" );
    while( 1 );
  }
  Println( "LoRa radio init OK!" );

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if( !rf95.setFrequency( RF95_FREQ ) )
  {
    Println( "setFrequency failed" );
    while( 1 );
  }
  Printchrs( "Set Freq to: " );
  Println( RF95_FREQ );

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
  digitalWrite( LED, HIGH );
  Println( "Send..." );

  _doSend = false;
  char *cursor = _sending_buff;
  memset(_sending_buff, '\0', sizeof(_sending_buff));

  _sendCounter++;
  Printchrs( "Send Count: ");
  Println( _sendCounter, DEC );
  // buff is METRIC_MSG_SIZE (should be 210)
  // '$' + '\t' + _node_name (max lenth of 10) + ':' + RSSI (16 bit int -> 5 digits plus a sign, yes RSSI is signed) + ':' + send count (16 bit int -> 5 digits) + '\t' -> 25 chars
  // each metric: name (max 10) + ':' + value ( 16 bit int ) + '\t' -> 18 chars
  // '$' + '\0' -> 2 chars 
  // 27 chars for start/end + 10 metrics at 18 = 207
  cursor += snprintf( cursor, sizeof( _sending_buff ), "$\t%s:%i:%i:%i\t", SENDER_NAME, rf95.lastRssi(), _sendCounter, _syncCounter );
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
  Println(_sending_buff);
  rf95.waitCAD();
  rf95.send( (uint8_t*) _sending_buff, strlen(_sending_buff) );
  rf95.waitPacketSent();

  if( !( _sendCounter % 60 ) )
    _doSync = true;

  Println( "Sent" );
  digitalWrite( LED, LOW );
}

void _setDoSend()
{
  _doSend = true;
}

void setupSender()
{
  uint8_t wrk;
  int name_len = strlen( SENDER_NAME );
  for( int i = 0; i < name_len; i++ )
    wrk += (unsigned char) SENDER_NAME[i]; // yes this will overflow the uint8, that is fine, we are "hashing"
  
  _offset = wrk % SLICE_COUNT;
  Printchrs("Offset: ");
  Println(_offset, DEC);

  rtc.begin();
  rtc.setTime( 0, 0, 0 );
  rtc.setAlarmSeconds( _offset );
  rtc.attachInterrupt( _setDoSend );
}

void sync()
{
  char buff[20];
  uint8_t len;
  rtc.disableAlarm();
  memset( buff, '\0', sizeof( buff ) );
  Println( "___ waiting for sync ___" );
  uint16_t counter = 0;
  while( 1 )
  {
    while( !rf95.available() )
    {
      digitalWrite( LED, counter < 2000 );
      counter++;
    }
    len = sizeof( buff );
    if( !rf95.recv( (uint8_t*) buff, &len ) )
      continue;

    Printchrs( "Len: " );
    Println(len, DEC );
    Printchrs( "Got: " );
    Println( (char*) buff );
    Printchrs( "RSSI: " );
    Println( rf95.lastRssi(), DEC );

    if( ( len == strlen( SYNC_MSG ) ) && ( strncmp( buff, SYNC_MSG, len ) == 0 ) )
      break;
  }
  _doSync = false;
  _syncCounter++;
  Println( "___ Synced ___" );
  rtc.setTime( 0, 0, 0 );
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
  if( _doSend )
    sendValues();

  if( _doSync ) // every 60 min
    sync();
}

size_t Println(const char c[])
{
  if( !doPrint )
    return 0;

  return Serial.println(c);
}

size_t Println(double num, int digits)
{
  if( !doPrint )
    return 0;

  return Serial.println(num, digits);
}

size_t Println(unsigned char b, int base)
{
  if( !doPrint )
    return 0;

  return Serial.println(b, base);
}

size_t Println(int num, int base)
{
  if( !doPrint )
    return 0;

  return Serial.println(num, base);
}

size_t Printchrs(const char c[])
{
  if( !doPrint )
    return 0;

  return Serial.print(c);
}
