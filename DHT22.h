#ifndef _DHT11
#define _DHT11 1

#include <Arduino.h>
#include "pins.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Message.h"

extern void startDHT();
extern uint16_t dhtGetHumidity();
extern float dhtGetTemp();
extern uint16_t dhtGetTemp10();
extern void showDhtState();
extern char getDhtStateSymbol();
extern int timer3count;
extern uint32_t statesEntered;
extern uint16_t humidityLow;

#endif

