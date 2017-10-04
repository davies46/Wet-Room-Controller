#include <Arduino.h>
#include "Message.h"
#include "Timer4.h"

uint16_t timer4count;
uint16_t timer4Start;
void (*timer4handler)(void) = NULL;

void timer4initMs(long ms, void (*handler)(void)) {
	timer4delayMs(ms);
	timer4handler = handler;
}

//Timer4 Overflow Interrupt Vector, called with varying times
void timer4delayMs(long ms) {
	timer4count = 0;
	TCCR4B = 0x00; //Disable Timer4 while we set it up
	TCNT4 = timer4Start = 65536l - 16l * ms; //Preset Timer Count
	TIFR4 = 0x00; //Timer4 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK4 = 0x01; //Timer4 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR4A = 0x00; //Timer4 Control Reg A: Wave Gen Mode normal
	TCCR4B = 0x05; //16kHz (64us per tick)
}

void timer4delayUs(long us) {
	timer4count = 0;
	TCCR4B = 0x00; //Disable Timer3 while we set it up
	TCNT4 = timer4Start = 65536l - us * 2; //Preset Timer Count
	TIFR4 = 0x00; //Timer3 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK4 = 0x01; //Timer3 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR4A = 0x00; //Timer3 Control Reg A: Wave Gen Mode normal
	TCCR4B = 0x02; //2MHz (0.5us per tick)
}

void stopTimer4(){
	TCCR4B = 0x00; //Disable Timer
	timer4handler=NULL;
}

ISR(TIMER4_OVF_vect) {
	timer4count++;
	TCNT4 = timer4Start; //Preset Timer Count to 130 out of 255
	if (timer4handler != NULL)
		timer4handler();
}

