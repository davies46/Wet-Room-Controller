/*
 * Pressure.cpp
 *
 *  Created on: 27 Aug 2014
 *      Author: pdavies
 */
#include <Arduino.h>
#include "Pins.h"
#include "Pressure.h"
#include "Constants.h"
#include "sharedVars.h"
#include "EepromUtil.h"

#define BMP_SAMPLEBUFFER_SIZE 10

uint32_t spuriousPressureDifferentialCount;
uint32_t maxAbsSpuriousPressureDif;

Pressure::Pressure() {
	// TODO Auto-generated constructor stub
}

Pressure::~Pressure() {
	// TODO Auto-generated destructor stub
}

void Pressure::init() {
	totalReadTime = totalReads = 0;
	waterPressureSensor = Adafruit_BMP085_Unified(10085);
	ambientPressureSensor = Adafruit_BMP085_Unified(10086);
	pressureDifferentials = new SamplesBuffer(BMP_SAMPLEBUFFER_SIZE);
	startupWaterPressure = startupAmbientPressure = bmpPressure = currentWaterPressure = currentAmbientPressure =
			meanPressureDifferential = lowestPressureDifferential = highestPressureDifferential = 0.0;
	maxAbsSpuriousPressureDif = 0;
	waterTempB5 = airTempB5 = 0;
	bothPresent = false;

	calibratePressureSensors();
	spuriousPressureDifferentialCount = 0;
}

void Pressure::calibratePressureSensors() {

//	timer4initMs(getOptionValue(OPT_GLITZ_DELAY), cycleButtonLights);

	selectBMP(AMBIENT_PRESSURE_SENSOR);
	ambientSensorPresent = ambientPressureSensor.begin();
	if (ambientSensorPresent) {
		Serial.println("ambient BMP085 found: ");
	} else {
		Serial.println("ambient BMP085 not found");
	}

	selectBMP(WATER_PRESSURE_SENSOR);
	waterSensorPresent = waterPressureSensor.begin();
	if (waterSensorPresent) {
		Serial.println("water BMP085 found: ");
	} else {
		Serial.println("water BMP085 not found");
	}

	bothPresent = waterSensorPresent && ambientSensorPresent;

	float lowW = 1000000.0, lowA = 1000000.0, highW = 0, highA = 0;
	if (bothPresent) {
		float totalAmbient = 0.0;
		float totalWater = 0.0;
		const int NUM_CALIBRATION_SAMPLES = 16;
		for (int i = 0; i < NUM_CALIBRATION_SAMPLES; i++) {
			readPressure(AMBIENT_PRESSURE_SENSOR);
			float t = ambientPressureSensor.getStoredTemperature();
			Serial.print(" rawA:");
			Serial.print(bmpPressure);
			Serial.print(", t:");
			Serial.print(t);

			lowA = min(lowA,bmpPressure);
			highA = max(highA,bmpPressure);
			totalAmbient += bmpPressure;

			readPressure(WATER_PRESSURE_SENSOR);
			t = waterPressureSensor.getStoredTemperature();
			Serial.print(" rawW:");
			Serial.print(bmpPressure);
			Serial.print(", t:");
			Serial.print(t);

			lowW = min(lowW,bmpPressure);
			highW = max(highW,bmpPressure);
			totalWater += bmpPressure;
			Serial.println();
		}
		startupAmbientPressure = totalAmbient / NUM_CALIBRATION_SAMPLES;
		startupWaterPressure = totalWater / NUM_CALIBRATION_SAMPLES;
		Serial.print("Low/High/Sprd Air: ");
		Serial.print(lowA);
		Serial.print(",");
		Serial.print(highA);
		Serial.print(",");
		Serial.println(highA - lowA);
		Serial.print("Low/High/Sprd Wtr: ");
		Serial.print(lowW);
		Serial.print(",");
		Serial.print(highW);
		Serial.print(",");
		Serial.println(highW - lowW);
		tMinLastCalibration = ticks.getMillis() / 60 / 1000;
	} else {
		Serial.println("Simulating pressure readings");
		meanPressureDifferential = 12.34;
	}
//	stopTimer4();
}

float Pressure::measurePressureDifferential() {
	if (waterSensorPresent) {
		readPressure(WATER_PRESSURE_SENSOR);
		drainTemp = bTemp;
		currentWaterPressure = bmpPressure - startupWaterPressure;
	}

	if (ambientSensorPresent) {
		readPressure(AMBIENT_PRESSURE_SENSOR);
		airTemp = bTemp;
		currentAmbientPressure = bmpPressure - startupAmbientPressure;
	}
	if (!bothPresent)
		return meanPressureDifferential;

	//	Serial.print("Air ");
	//	Serial.print(currentAmbientPressure);
	//	Serial.print("Drain ");
	//	Serial.println(currentWaterPressure);

	instantaneousPressureDifferential = currentWaterPressure - currentAmbientPressure;
	uint32_t absPressureDif = abs(instantaneousPressureDifferential);
	if (absPressureDif > MAX_SANE_PRESSURE_DIFFERENTIAL) {
		//spurious, ignore
		spuriousPressureDifferentialCount++;
		maxAbsSpuriousPressureDif = max(maxAbsSpuriousPressureDif, absPressureDif);
	} else {
		lowestPressureDifferential = min(instantaneousPressureDifferential, lowestPressureDifferential);
		highestPressureDifferential = max(instantaneousPressureDifferential, highestPressureDifferential);

		meanPressureDifferential = pressureDifferentials->addSample(instantaneousPressureDifferential);
	}
	return meanPressureDifferential;
}

void Pressure::readPressure(bool sensor) {
	selectBMP(sensor);
	uint64_t startTime = micros();
	((sensor == WATER_PRESSURE_SENSOR) ? waterPressureSensor : ambientPressureSensor).getPressureAndTemp(&bmpPressure, &bTemp);
	uint64_t readTime = micros() - startTime;
	totalReadTime += readTime;
	totalReads++;
}

void Pressure::selectBMP(bool s) {
	if (s == AMBIENT_PRESSURE_SENSOR) {
		digitalWrite(PIN_BMP_XCLR_WATER, 0);
		digitalWrite(PIN_BMP_XCLR_AIR, 1);
	} else {
		digitalWrite(PIN_BMP_XCLR_AIR, 0);
		digitalWrite(PIN_BMP_XCLR_WATER, 1);
	}
	delay(25);
}

void Pressure::reportStuff() {
	if (bothPresent) {
		waterTempB5 = airTempB5 = 0;
		selectBMP(WATER_PRESSURE_SENSOR);
		waterTempB5 = waterPressureSensor.readB5();
		selectBMP(AMBIENT_PRESSURE_SENSOR);
		airTempB5 = ambientPressureSensor.readB5();

		Serial.print(" low:");
		Serial.print(lowestPressureDifferential);
		Serial.print(" hi:");
		Serial.print(highestPressureDifferential);

		Serial.print(" wT:");
		Serial.print(waterPressureSensor.convertB5Temp(waterTempB5));
		Serial.print(" aT:");
		Serial.print(ambientPressureSensor.convertB5Temp(airTempB5));

		Serial.print(" RdT:");
		Serial.print((uint32_t) (totalReadTime / totalReads));

		Serial.println();
	}
}

