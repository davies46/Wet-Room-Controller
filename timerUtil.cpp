#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timerUtil.h"

void setPwmFrequency(int pin, int divisor) {
//2-13 44-46
//	timer 0 for pin 13 and 4
//	timer 1 for pin 12 and 11
//	timer 2 for pin 10 and 9
//	timer 3 for pin 5 and 3 and 2
//	timer 4 for pin 8 and 7 and 6

	byte mode;
	if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch (divisor) {
		case 1:
			mode = 0x01;
			break;
		case 8:
			mode = 0x02;
			break;
		case 64:
			mode = 0x03;
			break;
		case 256:
			mode = 0x04;
			break;
		case 1024:
			mode = 0x05;
			break;
		default:
			return;
		}
		if (pin == 5 || pin == 6) {
			TCCR0B = (TCCR0B & 0b11111000) | mode;
		} else {
			TCCR1B = (TCCR1B & 0b11111000) | mode;
		}
	} else if (pin == 3 || pin == 11) {
		switch (divisor) {
		case 1:
			mode = 0x01;
			break;
		case 8:
			mode = 0x02;
			break;
		case 32:
			mode = 0x03;
			break;
		case 64:
			mode = 0x04;
			break;
		case 128:
			mode = 0x05;
			break;
		case 256:
			mode = 0x06;
			break;
		case 1024:
			mode = 0x7;
			break;
		default:
			return;
		}
		TCCR2B = (TCCR2B & 0b11111000) | mode;
	}
}
