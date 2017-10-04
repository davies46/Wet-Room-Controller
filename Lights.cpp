/*
 * Lights.cpp
 *
 *  Created on: 18 Nov 2014
 *      Author: pdavies
 */

#include "Lights.h"
#include "EepromUtil.h"
#include "Timer4.h"

Lights::Lights() {
	ticksToAlcoveLightsOut = 255;
	ticksToShowerLightsOut = 255;
	alarm = false;
}

Lights::~Lights() {
	// TODO Auto-generated destructor stub
}

void Lights::switchAllOn() {
	ticksToShowerLightsOut = getOptionValue(OPT_SHOWER_LIGHTS_STAY);
	ticksToAlcoveLightsOut = getOptionValue(OPT_ALCOVE_LIGHTS_STAY);
}
void Lights::switchAlcoveOn() {
	ticksToAlcoveLightsOut = getOptionValue(OPT_ALCOVE_LIGHTS_STAY);
}
void Lights::switchOverheadOn() {
	ticksToShowerLightsOut = getOptionValue(OPT_SHOWER_LIGHTS_STAY);
}

void Lights::setBrightness() {
	if (alarm) {
		bool st = ticks.getMillis() / 500 & 1;
		uint8_t ast = st ? 255 : 0;
		analogWrite(PIN_ALCOVELIGHTS_PWM, ast);
		analogWrite(PIN_SHOWERLIGHTS_PWM, ast);
		buttonLights.buttonLight(BL_LIGHTS, st);
	} else {
		analogWrite(PIN_ALCOVELIGHTS_PWM, min(255, ticksToAlcoveLightsOut));
		analogWrite(PIN_SHOWERLIGHTS_PWM, min(255, ticksToShowerLightsOut));
		buttonLights.buttonLight(BL_LIGHTS, ticksToAlcoveLightsOut > 0 || ticksToShowerLightsOut > 0);
	}
}
