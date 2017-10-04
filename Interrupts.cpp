/*
 * Interrupts.cpp
 *
 *  Created on: 19 Nov 2014
 *      Author: pdavies
 */

#include "Interrupts.h"
#include "pins.h"
#include "Constants.h"
#include "sharedVars.h"
#include "EepromUtil.h"

uint16_t btnEventCnt;
bool btn1state;
uint64_t lastKeyUp, lastKeyDn;
uint64_t dwell;
int16_t dialTicks;
int16_t lastDialTicks;
int16_t dialCnt;
int16_t lastDialCnt;
uint32_t pcInt0Cnt = 0;
uint32_t pcInt1Cnt = 0;
uint32_t pc0IntCnt = 0;
uint32_t pc1IntCnt = 0;
bool pinDialA = 1, pinDialB = 1;
bool newPinA = 1, newPinB = 1;

void btnEvent();
void setupSpecialInterrupts();

Interrupts::Interrupts() {
	btnEventCnt = 0;
	btn1state = 1;
	dialCnt = lastDialCnt = 0;
	lastDialTicks = dialTicks = getOptionValue(OPT_UNUSED1) * 4;
	dialCnt = dialTicks / 4;
	lastKeyDn=ticks.getMillis();
	attachInterrupt(INTERRUPT_LINE_BTN1, btnEvent, CHANGE);
	setupSpecialInterrupts();
}

Interrupts::~Interrupts() {
	// TODO Auto-generated destructor stub
}

void Interrupts::resetInterruptCounts() {
	btnEventCnt = 0;
	pcInt0Cnt = 0;
	pcInt1Cnt = 0;
	dialTicks = 0;
	maxDeltaFlowPulses = 0;
}

void Interrupts::tick() {
	dialCnt = dialTicks / 4;
	if (dialCnt != lastDialCnt) {
		bool fwd = (dialCnt > lastDialCnt);
		lastDialTicks = dialTicks;
		lastDialCnt = dialCnt;
		oled->dial(fwd);
	}
	if (btn1state == 0) {
		//down... how long?
		if (ticks.howLongAgoWas(lastKeyDn) > MS_LONG_KEYPRESS) {
			Serial.println("Long keypress");
			oled->cycleDisplayMode();
			btn1state = 1;
		}
	}
}

/*
 * Generate the kepress event on release so we can generate a long or short keypress event.
 */
void btnEvent() {
	btnEventCnt++;
//push btn interrupt - but could be anything
//can we really track the state by monitoring the number of changes, or do we need to read the pin?
	bool btn = digitalRead(PIN_BTN_DIAL);
	if (btn != btn1state) {
		//only handle btn changes
		btn1state = btn;
		if (btn1state) {
			//key up
			Serial.println("up");
			lastKeyUp = ticks.getMillis();
			dwell = lastKeyUp - lastKeyDn;
			if (dwell > MS_SHORT_KEYPRESS && dwell < MS_LONG_KEYPRESS) {
				Serial.println("Short");
				messageQueue.sendMessage(Event::KEYPRESS_SHORT);
			}
		} else {
			//key down
			Serial.println("dn");
			lastKeyDn = ticks.getMillis();
		}
	}
}

/*
 * All this crap below for the dial, and it doesn't even work very well!
 */
void setupSpecialInterrupts() {
//set 3..0 to change, and 7..4
	EICRA = 0x55;
	EICRB = 0x55;
	EIMSK = 0xFF; //all ints enabled

//	PB0 ( SS/PCINT0 )	Digital pin 53 (SS)
//	PJ0 ( RXD3/PCINT9 )	Digital pin 15 (RX3)
	PCICR = 3; // (1 << PCIF0) | (1 << PCIF1); //enable iu on change PCINT7:0 and PCINT15:8
	PCMSK0 = 1; //1 << PCINT0;
	PCMSK1 = 2; //1 << PCINT9;
}

ISR(PCINT0_vect) {
//if A goes off when B on ++
//if A goes on when B on --
//if B goes off when A off ++
	pc0IntCnt++;
	newPinA = (PORT_DIAL_A & MASK_DIAL_A) != 0;
	newPinB = (PORT_DIAL_B & MASK_DIAL_B) != 0;

	if (pinDialA != newPinA) { //check if changed
		pcInt0Cnt++;
		dialTicks += ((pinDialA == pinDialB) ? 1 : -1);
	}
	pinDialA = newPinA; //flip if changed
	pinDialB = newPinB;
}

ISR(PCINT1_vect) {
//if A goes off when B on ++
//if A goes on when B on --
//if B goes off when A off ++
	newPinA = (PORT_DIAL_A & MASK_DIAL_A) != 0;
	newPinB = (PORT_DIAL_B & MASK_DIAL_B) != 0;

	pc1IntCnt++;

	if (pinDialB != newPinB) {
		pcInt1Cnt++;
		dialTicks += ((pinDialA == pinDialB) ? -1 : 1);
	}
	pinDialA = newPinA; //flip if changed
	pinDialB = newPinB;
}

