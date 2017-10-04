/*
 * ButtonLights.h
 *
 *  Created on: 9 Dec 2014
 *      Author: pdavies
 */

#ifndef BUTTONLIGHTS_H_
#define BUTTONLIGHTS_H_

#include "pins.h"
#include "Constants.h"

#define BL_RESET 0
#define BL_EXTRACTOR 1
#define BL_PUMP 2
#define BL_LIGHTS 3
#define BL_WAND 4
#define BL_OVERHEAD 5

#define BUTTONLIGHT_RESET (1<<BL_RESET)
#define BUTTONLIGHT_EXTRACTOR (1<<BL_EXTRACTOR)
#define BUTTONLIGHT_PUMP (1<<BL_PUMP)
#define BUTTONLIGHT_LIGHTS (1<<BL_LIGHTS)
#define BUTTONLIGHT_WAND (1<<BL_WAND)
#define BUTTONLIGHT_OVERHEAD (1<<BL_OVERHEAD)

#define ERROR_DHT_TIMEOUT_LOW (BUTTONLIGHT_EXTRACTOR|BUTTONLIGHT_LIGHTS)
#define ERROR_DHT_TIMEOUT_HIGH (BUTTONLIGHT_EXTRACTOR|BUTTONLIGHT_PUMP)
#define ERROR_DHT_CHECKSUM (BUTTONLIGHT_EXTRACTOR|BUTTONLIGHT_RESET)
#define ERROR_TIMER4 (BUTTONLIGHT_EXTRACTOR)

class ButtonLights {
private:
	void startTimer();
	bool resetButtonLightsState;
	bool extractorButtonLightsState;
	bool drainButtonLightsState;
	bool lightingButtonLightsState;
	uint8_t buttonLightStates[6];
	uint8_t buttonLightPins[6];
	void effectButtonLights();

public:
	ButtonLights();
	virtual ~ButtonLights();

	void startCycling();
	void stopCycling();
	void errorCode(uint8_t code);
	void clearError();

	void buttonLight(int light, bool s);
};

#endif /* BUTTONLIGHTS_H_ */
