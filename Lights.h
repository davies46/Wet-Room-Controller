/*
 * Lights.h
 *
 *  Created on: 18 Nov 2014
 *      Author: pdavies
 */

#ifndef LIGHTS_H_
#define LIGHTS_H_

#include "pins.h"

#define countdownTick(v) if (v > 0) v -= 1; if (v < 0) v = 0

class Lights {
	bool alarm;

	void setBrightness();

public:
	int32_t ticksToAlcoveLightsOut;
	int32_t ticksToShowerLightsOut;

	Lights();
	virtual ~Lights();
	void switchAllOn();
	void switchAlcoveOn();
	void switchOverheadOn();

	void tick() {
		countdownTick(ticksToAlcoveLightsOut);
		countdownTick(ticksToShowerLightsOut);
		setBrightness();
	}

	void setAlarm(bool a) {
		alarm = a;
	}
	bool getAlarm() {
		return alarm;
	}
};

#endif /* LIGHTS_H_ */
