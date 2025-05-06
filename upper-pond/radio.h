#ifndef RADIO_H
#define RADIO_H

#define LED 13

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// ideally it should go into 256 (uint8_t) evenily
#define SLICE_COUNT 128

// The MAX size is 240 something
#define METRIC_MSG_SIZE 200

struct value_entry
{
  char name[50];
  int16_t *value;
  bool *dirty;
};

void radio_setup( int power );
RH_RF95 *getRF95();
void setupSender( const char *name, const value_entry *values );
void senderProcess();
void sync();
void sendSync();
void sendBasic(int value_level, int value_battery);

#endif