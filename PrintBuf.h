/*
 * PrintBuf.h
 *
 *  Created on: 27 Dec 2014
 *      Author: pdavies
 */

#ifndef PRINTBUF_H_
#define PRINTBUF_H_

#include <Arduino.h>

class PrintBuf: public Print {
	uint8_t buf[128];
	char *bufP;
	uint8_t lastLen;

public:
	inline PrintBuf() {
		bufP = (char*) buf;
	}
	virtual ~PrintBuf() {
	}

	inline char *consume() {
		//put eol at end then return bufP to start
		*bufP++ = '\0';
		lastLen = bufP - (char*) buf;
		bufP = (char*) buf;
		return bufP;
	}

	inline uint8_t getLen() {
		return lastLen;
	}

	inline size_t write(uint8_t ch) {
		*bufP++ = ch;
		return 1;
	}

	inline size_t write(const uint8_t *buffer, size_t size) {
		const char* srcP = (char*) buffer;
		strcpy(bufP, srcP);
		bufP += size;
		return size;
	}
};

#endif /* PRINTBUF_H_ */
