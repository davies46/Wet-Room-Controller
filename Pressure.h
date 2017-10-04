/*
 * Pressure.h
 *
 *  Created on: 27 Aug 2014
 *      Author: pdavies
 */

#ifndef PRESSURE_H_
#define PRESSURE_H_

#include "SamplesBuffer.h"
#include "Adafruit_BMP085_U.h"
#include "Constants.h"

#define AMBIENT_PRESSURE_SENSOR 0
#define WATER_PRESSURE_SENSOR 1
#define ESTIMATED_PRESSURE_AT_LOWER_FLOAT 20.0

class Pressure {
	Adafruit_BMP085_Unified waterPressureSensor;
	Adafruit_BMP085_Unified ambientPressureSensor;

	float startupWaterPressure;
	float startupAmbientPressure;
	float bmpPressure;
	float bTemp;
	float airTemp;
	float drainTemp;
	float currentWaterPressure;
	float currentAmbientPressure;
	float meanPressureDifferential;
	float lowestPressureDifferential;
	float highestPressureDifferential;
	int32_t waterTempB5;
	int32_t airTempB5;

	SamplesBuffer *pressureDifferentials;

	uint64_t totalReadTime;
	uint32_t totalReads;

	bool bothPresent;
	bool ambientSensorPresent;
	bool waterSensorPresent;

	void readPressure(bool sensor);
	void selectBMP(bool s);

public:
	float instantaneousPressureDifferential;

	Pressure();
	virtual ~Pressure();

	void init();
	void calibratePressureSensors();
	void syncAtLowerFloat() {
		startupAmbientPressure = currentAmbientPressure;
		startupWaterPressure = currentWaterPressure - ESTIMATED_PRESSURE_AT_LOWER_FLOAT;
	}
	float measurePressureDifferential();
	float getMeanPressureDifferential() {
		return meanPressureDifferential;
	}
	float getInstantaneousPressureDifferential() {
		return instantaneousPressureDifferential;
	}
	float getAirPressure() {
		return currentAmbientPressure;
	}
	float getDrainPressure() {
		return currentWaterPressure;
	}
	void reportStuff();
	bool present() {
		return bothPresent;
	}
	bool airPresent() {
		return ambientSensorPresent;
	}
	bool wtrPresent() {
		return waterSensorPresent;
	}
	void setFakeDifferential(float fkp) {
		meanPressureDifferential = fkp;
	}
	float getDrainTemp() {
		return drainTemp;
	}
	float getAirTemp() {
		return airTemp;
	}
};

#endif /* PRESSURE_H_ */
