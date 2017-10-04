/*
 * Pump.h
 *
 *  Created on: 11 Nov 2014
 *      Author: pdavies
 */
#include <Arduino.h>
#include "Constants.h"
#include "sharedVars.h"

#ifndef PUMP_H_
#define PUMP_H_

class Pump {
private:
	bool enabled;
	bool offActionsDone;
	uint64_t secondPumpStopped;

public:
	int16_t leakyBucket;
	int16_t prevBucket;
	Pump() {
		prevBucket = leakyBucket = 0;
		enabled = true;
		secondPumpStopped = 0;
	}

	virtual ~Pump() {
	}

	/**
	 * Capped at 512
	 */
	void requestMinSpeed(int16_t speed) {
		prevBucket = leakyBucket;
		leakyBucket = min(512, max(leakyBucket, speed));
		if (prevBucket == 0 && leakyBucket > 0) {
			offActionsDone = false;
		}
	}

	bool isRunning() {
		return getEffectiveMotorSpeed() > 0;
	}

	uint16_t getEffectiveMotorSpeed() {
		return enabled ? leakyBucket : 0;
	}

	bool doOffActions(uint64_t numSeconds) {
		if (!offActionsDone && beenOffForSeconds() > numSeconds) {
			return offActionsDone = true;
		}
		return false;
	}

	uint64_t beenOffForSeconds();

	void tick();

	bool requested() {
		return leakyBucket > 0;
	}

	void enable() {
		enabled = true;
	}

	void disable() {
		enabled = false;
	}
};

#endif /* PUMP_H_ */
