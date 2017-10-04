/*
 * Diverter.cpp
 *
 *  Created on: 6 Oct 2014
 *      Author: pdavies
 */

#include "Constants.h"
#include "Diverter.h"
#include "sharedVars.h"
#include "pins.h"
#include "EepromUtil.h"

void Diverter::syncHardware() {
	Serial.print("Setting diverter output mode to ");
	Serial.println(currentMode);

	digitalWrite(PIN_OUTPUT_OVERHEAD_A, wantOverhead);
	digitalWrite(PIN_OUTPUT_OVERHEAD_B, wantOverhead);

	digitalWrite(PIN_OUTPUT_WAND_A, !wantWand);
	digitalWrite(PIN_OUTPUT_WAND_B, !wantWand);

	setIndicators();
}

void Diverter::restoreFromEeprom() {
	setCurrentMode(getOptionValue(OPT_DIVERTER_STATE));
}

void Diverter::setIndicators() {
	buttonLights.buttonLight(BL_OVERHEAD, wantOverhead);
	buttonLights.buttonLight(BL_WAND, wantWand);
}

void Diverter::cycle() {
	abortPurge();
	Serial.print("Diverter from ");
	Serial.print(currentMode);
	setCurrentMode(nextOutputMode[currentMode]);
	Serial.print(" to ");
	Serial.println(currentMode);
	setAndSaveOptionValue(OPT_DIVERTER_STATE, currentMode);
}

void Diverter::tick() {
	uint64_t secondsSinceFlowStopped = flowMeter->timeStoppedInSeconds();
	if (!purgedSinceFlowStarted && secondsSinceFlowStopped > getOptionValue(OPT_PURGE_DELAY)) {
		Serial.println("Pending reset");
		if (diverterExecutingResetting) {
			//will come here during flush stage
			if (secondsSinceFlowStopped > getOptionValue(OPT_PURGE_TIME)) {
				setCurrentMode(Diverter::OUTPUT_OVERHEAD);
				diverterExecutingResetting = false;
				purgedSinceFlowStarted = true;
				setAndSaveOptionValue(OPT_DIVERTER_STATE, currentMode);
			}
		} else {
			//pending reset but not executing, set to flush
			setCurrentMode(Diverter::OUTPUT_BOTH_O);
			diverterExecutingResetting = true;
		}
	}
}

Diverter::Diverter() {
	Serial.println("Diverter INIT");
	nextOutputMode[OUTPUT_OVERHEAD] = OUTPUT_BOTH_W;
	nextOutputMode[OUTPUT_BOTH_W] = OUTPUT_WAND;
	nextOutputMode[OUTPUT_WAND] = OUTPUT_BOTH_O;
	nextOutputMode[OUTPUT_BOTH_O] = OUTPUT_OVERHEAD;

	purgedSinceFlowStarted = true;
	diverterExecutingResetting = false;

	setCurrentMode(getOptionValue(OPT_DIVERTER_STATE));
}

Diverter::~Diverter() {
	// TODO Auto-generated destructor stub
}

