/*
 * TX433.cpp
 *
 *  Created on: 23 Dec 2014
 *      Author: pdavies
 */

#include "RH_ASK.h"
#include "TX433.h"

RH_ASK *rhAsk;

TX433::TX433(const int rate, const int rxData, const int txData, const int enable, bool pttInvert) {
	this->rate = rate;
	this->rxData = rxData;
	this->txData = txData;
	this->enable = enable;
	this->pttInvert = pttInvert;
	rhAsk = NULL;
	initRhInstance();
	Serial.print("I");
}

void TX433::initRhInstance() {
	if (rhAsk != NULL) {
		delete rhAsk;
	}
	rhAsk = new RH_ASK(rate, rxData, txData, enable, pttInvert);
	rhAsk->init();
	rhAsk->setModeTx();
	consume();
}

void TX433::send() {
	char *b = consume();
	int len = getLen();
//	Serial.println();
//	Serial.print("Len=");Serial.print(len);
	rhAsk->send((uint8_t*) b, (uint8_t) len);
}

void TX433::speed(int speed) {
	this->rate = speed;
	initRhInstance();
}

TX433::~TX433() {
}

