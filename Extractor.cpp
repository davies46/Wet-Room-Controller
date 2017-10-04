/*
 * Extractor.cpp
 *
 *  Created on: 1 Nov 2014
 *      Author: pdavies
 */

#include <Arduino.h>
#include "Extractor.h"
#include "Constants.h"
#include "pins.h"
#include "sharedVars.h"
#include "EepromUtil.h"
#include "DHT22.h"

Extractor::Extractor() {
	ticksRemaining = 0;
	humidity = 0;
	flashState = 0;
	fanLastSwitchedOnAt = fanLastSwitchedOffAt = ticks.getMillis();
}

void Extractor::recordHumidity() {
//	Serial.print("DHT:");
	humidity = dhtGetHumidity();
//	Serial.println(humidity / 10.0);
}

void Extractor::setFanStay(uint16_t tks) {
	ticksRemaining = max(ticksRemaining, tks*2);
}

void Extractor::actionExtractorFan() {

	bool onState = ticksRemaining != 0;

	//whether the fan is on or off depends on whether there are ticks remaining, i.e. onState
	digitalWrite(PIN_EXTRACTOR_FAN, onState ? OC_ON : OC_OFF);

	/*
	 * Usually we set the button light according to the fan state, but maybe we should do:
	 * humidity > thr -> fan on, light on
	 * humidity < thr, isRunning() -> fan on, light flashing
	 * humidity < thr, not running -> fan off light off
	 */
	if (inCooldown()) {
		onState = flashState;
	}
	buttonLights.buttonLight(BL_EXTRACTOR, onState);
}
bool Extractor::atOrAboveThreshold() {
	return (humidity >= getOptionValue(OPT_HUMIDITY_THRESHOLD));
}

bool Extractor::belowThreshold() {
	return (humidity < getOptionValue(OPT_HUMIDITY_THRESHOLD));
}

bool Extractor::inCooldown() {
	return belowThreshold() && isRunning();
}

void Extractor::tick() {
	flashState = !flashState;
	if (ticksRemaining > 0) {
		ticksRemaining--;
		if (ticksRemaining == 0) {
			fanLastSwitchedOffAt = ticks.getMillis();
		}
	} else
		ticksRemaining = 0;
}

void Extractor::checkExtractor() {
	if (humidity > getOptionValue(OPT_HUMIDITY_THRESHOLD)) {
		//make fan stay on for a minimum of EXTRACTOR_DWELL to stop it switching on for only 1s
		start(2);
	}
}

/**
 * time in seconds
 */
void Extractor::start(int d) {
	// ignored if was recently running
	long int fanLastRanAgo = ticks.getMillis() - fanLastSwitchedOffAt;
	bool canSwitchOnAgainNow = fanLastRanAgo > 1000 * FAN_OFFTIME;
	if (canSwitchOnAgainNow) {
		if (ticksRemaining == 0) {
			fanLastSwitchedOnAt = ticks.getMillis();
		}
		setFanStay(getOptionValue(OPT_EXTRACTOR_DWELL) * 2);
	}
}

void Extractor::stop() {
	setFanStay(0);
}

void Extractor::recalibrateToChangeState() {
	int16_t delta = getOptionValue(OPT_HUMIDITY_CAL);
	setAndSaveOptionValue(OPT_HUMIDITY_THRESHOLD, humidity + (atOrAboveThreshold() ? +delta : -delta)); // make sure it's good to stop or go now
	checkExtractor();
}
