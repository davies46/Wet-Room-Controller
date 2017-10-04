#ifndef Buffer_h
#define Buffer_h

#include <Arduino.h>

#define BUF_SIZE 256

byte rxBuf[BUF_SIZE];
int bufPtr = 0;
int total = 0;

void addSample(byte sample) {
	int nxtPtr = (bufPtr + 1) % BUF_SIZE;
	total += (sample - rxBuf[nxtPtr]);
	rxBuf[bufPtr] = sample;
	bufPtr = nxtPtr;
}

int getMean() {
	return total / BUF_SIZE;
}

#endif

