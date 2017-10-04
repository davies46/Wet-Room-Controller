/*
 * Buttons.cpp
 *
 *  Created on: 19 Nov 2014
 *      Author: pdavies
 */

#include "Constants.h"
#include "EepromUtil.h"
#include "sharedVars.h"
#include "pins.h"
#include "Buttons.h"

#define MAX_FAN_MS (1000 * 60 * 30)

Buttons::Buttons() {
	divertorBtnState = extractorBtnState = BUTTON_RELEASED;
	timeExtractorButtonWasFirstPressed = 0;
	lastDiverterBtnAction = ticks.getMillis();
}

Buttons::~Buttons() {
}

void Buttons::check() {
	bool anyBtnPressed = false;
	//there are only 3 control panel buttons we need to handle because the reset is outside of our control.
	if (digitalRead(PIN_BUTTON_LIGHTS) == BUTTON_PRESSED) {
		lights.switchAllOn();
		anyBtnPressed = true;
	}

	//extractor fan. if the button's down we want the fan to run.
	//if the button comes up after a long while then we switch the fan off
	//if the button comes up fast then we want to do the toggle

	if (ticks.howLongAgoWas(timeExtractorButtonWasFirstPressed) > 10) { //(usually is >10ms). if really short, we're in contact bounce - ignore
		if (digitalRead(PIN_BUTTON_EXTRACTOR) == BUTTON_PRESSED) { //the button's down, what's the recorded state?
			if (extractorBtnState == BUTTON_RELEASED) { // recorded state was up, so here we're marking the transition
				extractorBtnState = BUTTON_PRESSED; // record as down so we we won't come here again until something else changes
				timeExtractorButtonWasFirstPressed = ticks.getMillis(); //button went from released to pressed, note the time
				anyBtnPressed = true; //a button was pressed
			} else { //button still pressed. see if it's after the short press period
				if (ticks.howLongAgoWas(timeExtractorButtonWasFirstPressed) > 300) { //button still down after 300ms. could also be an hour :)
					//we'll come here loads if button held down. Got to start/restart the fan unless we've been fanning more than 30 mins (max time)
					if (ticks.howLongAgoWas(timeExtractorButtonWasFirstPressed)
							< (60 * 1000 * getOptionValue(OPT_MAXTIME_EXTRACTOR))) {
						extractor.start(1);
					}
				}
			}
		} else { //============== Button RELEASED
			if (extractorBtnState == BUTTON_PRESSED) { //was pressed so first time through. if it's a short press then we've got to toggle and recalibrate
													   //if long we shouldn't come here for hours
				extractorBtnState = BUTTON_RELEASED;
				if (ticks.howLongAgoWas(timeExtractorButtonWasFirstPressed) < 300) { //a third of second-ish is plenty
					extractor.recalibrateToChangeState(); //change humidity threshold so the state changes
				}
			}
		}
	}

	//the pump
	if (digitalRead(PIN_BUTTON_PUMP) == BUTTON_PRESSED) {
//		Serial.print("Btn sw");
		pump.requestMinSpeed(255);
		anyBtnPressed = true;
	}

//then the diverter button
#ifdef DEBOUNCE_NEEDED_ON_DIVERTER_BUTTON
	if (digitalRead(PIN_BUTTON_DIVERTER) != divertorBtnState) {
		//we've caught a state change. If it was too fast to catch, it's not problem
		divertorBtnState = !divertorBtnState;
		if (divertorBtnState == 0) {
			//it was a down press
			if (ticks.howLongAgoWas(lastDiverterBtnAction) > DEBOUNCE_MILLIS) {
				//first contact, action. cycle shower valve states
				diverter->cycle();
				lastDiverterBtnAction = ticks.getMillis();
				anyBtnPressed = true;
				analogWrite(PIN_ALCOVELIGHTS_PWM, 0); //blink alcove light off
			}
		}
	}
#else
	if (digitalRead(PIN_BUTTON_DIVERTER) == BUTTON_PRESSED) {
		//we've caught a state change. If it was too fast to catch, it's not problem
		cycleDiverter();
		anyBtnPressed = true;
	}
#endif

	if (anyBtnPressed) {
		lights.setAlarm(false);
	}
}

