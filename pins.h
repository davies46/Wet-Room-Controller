#ifndef _PINS
#define _PINS 1

#include "Arduino.h"

/*
 * outputs
 */

#define Output(pin,state) digitalWrite(pin,state); pinMode(pin, OUTPUT)

#define PIN_ANA_HUMIDITY1 A15
#define PIN_ANA_HUMIDITY2 A14

#define PIN_RL1 47
#define PIN_RL2 45
#define PIN_RL3 43
#define PIN_RL4 41
#define PIN_RL5 40
#define PIN_RL6 42
#define PIN_RL7 44
#define PIN_RL8 46

#define PIN_OC_A1 38
#define PIN_OC_B1 36
#define PIN_OC_C1 34
#define PIN_OC_D1 32
#define PIN_OC_A2 24
#define PIN_OC_B2 26
#define PIN_OC_C2 27
#define PIN_OC_D2 25

#define PIN_MOTOR1_PWM 10 //connects to motor1 FET
#define PIN_MOTOR2_PWM 9 //connects to motor2 FET

#define PIN_EXTRACTOR_FAN PIN_OC_A2 // switches O/C that switches 12v Relay in store room.
#define PIN_POWERSHOWER_PUMP_ENABLE PIN_OC_B2 //connects to O/C then out of room via shower piping to SSR at shower pump

#define PIN_SHOWERLIGHTS_PWM 4 //connects to FET which supplies 12v to shower lights
#define PIN_ALCOVELIGHTS_PWM 11 //connects to FET which supplies 12v to alcove lights

#define PIN_LED 13

#define PIN_OUTPUT_OVERHEAD_A PIN_RL3
#define PIN_OUTPUT_OVERHEAD_B PIN_RL4
#define PIN_OUTPUT_WAND_A PIN_RL1
#define PIN_OUTPUT_WAND_B PIN_RL2

#define PIN_BMP_XCLR_AIR 31 //connect to XCLR (via o/c transistor?)
#define PIN_BMP_XCLR_WATER 30 //connect to XCLR (via o/c transistor?)

#define PIN_BUTTONLIGHT_RESET PIN_OC_A1
#define PIN_BUTTONLIGHT_EXTRACTOR PIN_OC_B1
#define PIN_BUTTONLIGHT_PUMP PIN_OC_C1
#define PIN_BUTTONLIGHT_LIGHTS PIN_OC_D1

#define PIN_BUTTONLIGHT_DIVERTER_OVERHEAD PIN_OC_D2
#define PIN_BUTTONLIGHT_DIVERTER_WAND PIN_OC_C2

#define PIN_433_TXDATA 7
#define PIN_433_TXENABLE 6
#define PIN_UNUSED 8

/*
 * inputs
 */
#define PIN_WATER_LEVEL_A 28 //connects directly to float switch (use PULLUP)
#define PIN_WATER_LEVEL_B 29 //connects directly to float switch (use PULLUP)

#define PIN_BUTTON_EXTRACTOR 33
#define PIN_BUTTON_PUMP 35
#define PIN_BUTTON_LIGHTS 37

#define PIN_BUTTON_DIVERTER 39

#define PIN_DIAL_A 53
#define PIN_DIAL_B 15

#define MASK_DIAL_A 1
#define MASK_DIAL_B 1

#define PORT_DIAL_A PINB
#define PORT_DIAL_B PINJ

/*
 * 2 (interrupt 0), 3 (interrupt 1), 18 (interrupt 5), 19 (interrupt 4), 20 (interrupt 3), and 21 (interrupt 2).
 */

/*
 * Input and output
 */
#define INTERRUPT_LINE_DHT 4
#define PIN_DHT_IO 19 //(iu 4)

#define INTERRUPT_LINE_FLOWMETER 5
#define PIN_SHOWER_FLOW_METER 18 //(iu 5)

#define INTERRUPT_LINE_BTN1 1
#define PIN_BTN_DIAL 3 //(iu 1)

//#define _USING_LIGHT_FOR_MOTOR 1
//#define _USING_DHT 1

#endif

