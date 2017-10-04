/*
 * Diverter.h
 *
 *  Created on: 6 Oct 2014
 *      Author: pdavies
 */

#ifndef DIVERTER_H_
#define DIVERTER_H_

#include <Arduino.h>

class Diverter {
private:
	bool purgedSinceFlowStarted;
	bool diverterExecutingResetting;
	uint8_t nextOutputMode[8];
	uint8_t currentMode;
	bool wantOverhead;
	bool wantWand;
	void setCurrentMode(uint8_t mode) {
		Serial.print("Setting diverter mode to ");
		Serial.println(mode);
		currentMode = mode;
		wantOverhead = (currentMode & OUTPUT_OVERHEAD) != 0;
		wantWand = (currentMode & OUTPUT_WAND) != 0;
		syncHardware();
	}
	void syncHardware();
	void setIndicators();
	void abortPurge() {
		purgedSinceFlowStarted = true;
		diverterExecutingResetting = false;
	}

public:
	static const uint8_t OUTPUT_OVERHEAD = 2;
	static const uint8_t OUTPUT_BOTH_W = 3;
	static const uint8_t OUTPUT_WAND = 1;
	static const uint8_t OUTPUT_BOTH_O = 7;

	Diverter();
	virtual ~Diverter();

	void cycle();
	void restoreFromEeprom();
	void tick();
	void primeForPurge() {
		purgedSinceFlowStarted = false;
	}
};

#endif /* DIVERTER_H_ */
