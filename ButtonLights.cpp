/*
 * ButtonLights.cpp
 *
 *  Created on: 9 Dec 2014
 *      Author: pdavies
 */

#include "ButtonLights.h"
#include "EepromUtil.h"
#include "Timer4.h"

uint8_t buttonDiagState;
bool buttonLightsCycling;
bool errorPresent;
uint8_t extractorFlashState;
uint8_t errCode;
bool errorOnOffState;

ButtonLights::ButtonLights() {
	buttonLightsCycling = false;
	errorPresent = false;
	errCode = 0;
	errorOnOffState = 0;
	buttonLightStates[BL_RESET] = OC_ON;
	buttonLightStates[BL_EXTRACTOR] = OC_OFF;
	buttonLightStates[BL_PUMP] = OC_OFF;
	buttonLightStates[BL_LIGHTS] = OC_OFF;
	buttonLightStates[BL_OVERHEAD] = OC_ON;
	buttonLightStates[BL_WAND] = OC_ON;

	buttonLightPins[BL_RESET] = PIN_BUTTONLIGHT_RESET;
	buttonLightPins[BL_EXTRACTOR] = PIN_BUTTONLIGHT_EXTRACTOR;
	buttonLightPins[BL_PUMP] = PIN_BUTTONLIGHT_PUMP;
	buttonLightPins[BL_LIGHTS] = PIN_BUTTONLIGHT_LIGHTS;
	buttonLightPins[BL_OVERHEAD] = PIN_BUTTONLIGHT_DIVERTER_OVERHEAD;
	buttonLightPins[BL_WAND] = PIN_BUTTONLIGHT_DIVERTER_WAND;

	effectButtonLights();
}

ButtonLights::~ButtonLights() {
}

void ButtonLights::effectButtonLights() {
	if (!buttonLightsCycling)
		for (int i = 0; i < 6; i++) {
			Output(buttonLightPins[i], buttonLightStates[i]);
		}
}

/*
 * Lights.cpp
 *
 *  Created on: 18 Nov 2014
 *      Author: pdavies
 */

void cycleButtonLights() {
	if (buttonLightsCycling) {
		digitalWrite(PIN_BUTTONLIGHT_RESET, buttonDiagState & 1);
		digitalWrite(PIN_BUTTONLIGHT_EXTRACTOR, buttonDiagState & 2);
		digitalWrite(PIN_BUTTONLIGHT_PUMP, buttonDiagState & 4);
		digitalWrite(PIN_BUTTONLIGHT_LIGHTS, buttonDiagState & 8);
		buttonDiagState = buttonDiagState * 2;
		if (buttonDiagState > 8) {
			buttonDiagState = 1;
		}
	}
	if (errorPresent) {
		uint8_t effectiveErrorCode = (errorOnOffState) ? errCode : 0;
		digitalWrite(PIN_BUTTONLIGHT_RESET, effectiveErrorCode & 1);
		digitalWrite(PIN_BUTTONLIGHT_EXTRACTOR, effectiveErrorCode & 2);
		digitalWrite(PIN_BUTTONLIGHT_PUMP, effectiveErrorCode & 4);
		digitalWrite(PIN_BUTTONLIGHT_LIGHTS, effectiveErrorCode & 8);
		errorOnOffState = 1 - errorOnOffState;
	}
}

void ButtonLights::buttonLight(int light, bool s) {
	buttonLightStates[light] = s;
	effectButtonLights();
}

void ButtonLights::startCycling() {
	buttonLightsCycling = true;
	buttonDiagState = 1;
	startTimer();
}

void ButtonLights::startTimer() {
	timer4initMs(getOptionValue(OPT_GLITZ_DELAY), cycleButtonLights);
}

void ButtonLights::stopCycling() {
	stopTimer4();
	buttonLightsCycling = false;
	effectButtonLights();
}

void ButtonLights::errorCode(uint8_t code) {
	errorPresent = true;
	errCode |= code;
	startTimer();
}

void ButtonLights::clearError() {
	stopTimer4();
	errCode = 0;
	errorPresent = false;
}

