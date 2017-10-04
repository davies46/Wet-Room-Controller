/*
 * SamplesBuffer.h
 *
 *  Created on: 20 Aug 2014
 *      Author: pdavies
 */

#ifndef SAMPLESBUFFER_H_
#define SAMPLESBUFFER_H_

#include <Arduino.h>

class SamplesBuffer {
private:
	float *buf;
	float total;
	uint16_t ptr;
	uint16_t sz;
public:
	SamplesBuffer(uint16_t size) {
		sz = size;
		buf = (float*) malloc(size * sizeof(float));
		for (uint16_t i = 0; i < size; i++) {
			buf[i] = 0.0;
		}
		ptr = 0;
		total = 0.0;
	}
	virtual ~SamplesBuffer();

	float addSample(float sample) {
		//add value into total and remove outgoing value
		total = total+ sample - buf[ptr];
		buf[ptr] = sample;
		ptr = (ptr + 1) % sz;
		return getMean();
	}

	float getMean() {
		return total / sz;
	}
};

#endif /* SAMPLESBUFFER_H_ */

