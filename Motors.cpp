/*
 * motors.cpp
 *
 *  Created on: 12 Nov 2014
 *      Author: pdavies
 */

#include "Motors.h"

#define NUM_SOFTSTART_CYCLES 50
#define SOFTSTART_POWER_STEP 5

void Motors::setSpeed(const uint16_t spd) {
	//allow quick down-change but smooth up-change.

	//  0-127: m1: 127-255
	//128-255: m2: 127-255
	int requestedM1Spd;
	int requestedM2Spd;
	if (spd < 128) {
		if (spd == 0) {
			requestedM1Spd = 0;
		} else {
			requestedM1Spd = 128 + spd;
		}
		requestedM2Spd = 0;
	} else {
		requestedM1Spd = 255;
		if (spd > 255) {
			requestedM2Spd = 255;
		} else {
			requestedM2Spd = spd;
		}
	}

	if (spd > 0) {
		tMinLastActivity = ticks.getMillis() / 1000 / 60;
	}

	int effectiveM1Spd = softStart(requestedM1Spd, prevM1Speed);
	int effectiveM2Spd = softStart(requestedM2Spd, prevM2Speed);

	analogWrite(PIN_MOTOR1_PWM, effectiveM1Spd);
	analogWrite(PIN_MOTOR2_PWM, effectiveM2Spd);
}

int Motors::softStart(int requestedSpd, uint16_t &prevSpeed) {
	int effectiveSpd;
	//Check if the motor1 is just starting
	if (prevSpeed == 0 && requestedSpd > 0 && softstartCycle == 0) {
		//begin the soft start cycle
		softstartCycle = 1;
	}

	if (softstartCycle > 0) {
		effectiveSpd = min(requestedSpd, softstartCycle * SOFTSTART_POWER_STEP);
		softstartCycle++;
		if (softstartCycle == NUM_SOFTSTART_CYCLES) {
			softstartCycle = 0;
		}
	} else {
		effectiveSpd = requestedSpd;
	}
	prevSpeed = effectiveSpd;
	return effectiveSpd;
}
