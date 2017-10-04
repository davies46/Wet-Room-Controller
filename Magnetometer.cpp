/*
 * Magnetometer.cpp
 *
 *  Created on: 14 Dec 2014
 *      Author: pdavies
 */

#include "EepromUtil.h"
#include "Magnetometer.h"

Magnetometer::Magnetometer(int id) {
	mag = Adafruit_HMC5883_Unified(id);
	if (mag.begin()) {
		magnetometerPresent = 1;
		calibrate();
	} else {
		/* There was a problem detecting the HMC5883 ... check your connections */
		Serial.println("Ooops, no HMC5883 magnetometer detected - is it connected?");
		magnetometerPresent = false;
	}
}

Magnetometer::~Magnetometer() {
}

void Magnetometer::calibrate() {
	if (isPresent()) {
		float rawMagFldTotal = 0.0;
		const int numSamples = 20;
		for (int i = 0; i < numSamples; i++) {
			bool ok = read();
			if (!ok) {
				Serial.println("Mag read FAIL on calib");
			}
			rawMagFldTotal += getX() - getY();
		}
		int16_t zeroAt = rawMagFldTotal / numSamples;
		Serial.print("Raw magfld baseline: ");
		Serial.println(zeroAt);
		setAndSaveOptionValue(OPT_MAG_CAL_OFS, zeroAt);
		Serial.print("Saved and restored as ");
		Serial.println((int16_t) getOptionValue(OPT_MAG_CAL_OFS));
	}
}

bool Magnetometer::isPresent() {
	return magnetometerPresent;
}
