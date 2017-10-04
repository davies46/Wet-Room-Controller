/*
 * Magnetometer.h
 *
 *  Created on: 14 Dec 2014
 *      Author: pdavies
 */

#ifndef MAGNETOMETER_H_
#define MAGNETOMETER_H_

#include "Adafruit_HMC5883_U.h"

class Magnetometer {
	bool magnetometerPresent;
	Adafruit_HMC5883_Unified mag;
	sensors_event_t event;
	sensor_t sensor;

public:
	Magnetometer(int id);
	virtual ~Magnetometer();
	bool isPresent();
	void calibrate();
	inline bool read() {
		return mag.getEvent(&event);
	}

	inline float getX() {
		return event.magnetic.x;
	}
	inline float getY() {
		return event.magnetic.y;
	}
	inline float getZ() {
		return event.magnetic.z;
	}
	inline void displaySensorDetails(void) {
		mag.getSensor(&sensor);
		Serial.println("------------------------------------");
		Serial.print("Sensor:       ");
		Serial.println(sensor.name);
		Serial.print("Driver Ver:   ");
		Serial.println(sensor.version);
		Serial.print("Unique ID:    ");
		Serial.println(sensor.sensor_id);
		Serial.print("Max Value:    ");
		Serial.print(sensor.max_value);
		Serial.println(" uT");
		Serial.print("Min Value:    ");
		Serial.print(sensor.min_value);
		Serial.println(" uT");
		Serial.print("Resolution:   ");
		Serial.print(sensor.resolution);
		Serial.println(" uT");
		Serial.println("------------------------------------");
		Serial.println("");
	}

};

#endif /* MAGNETOMETER_H_ */
