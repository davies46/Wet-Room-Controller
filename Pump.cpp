/*
 * Pump.cpp
 *
 *  Created on: 11 Nov 2014
 *      Author: pdavies
 */

#include "EepromUtil.h"
#include "Pump.h"

uint64_t Pump::beenOffForSeconds() {
	return ticks.ageInSeconds(secondPumpStopped);
}

void Pump::tick() {
	if (requested()) {
		leakyBucket -= getOptionValue(OPT_PUMP_LEAKY_BUCKET_LEAK_RATE);
		if (leakyBucket <= 0) {
			secondPumpStopped = ticks.getSecond();
		}
	}

	if (leakyBucket < 0)
		leakyBucket = 0;
}
