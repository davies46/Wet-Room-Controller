/*
 * TX433.h
 *
 *  Created on: 23 Dec 2014
 *      Author: pdavies
 */

#ifndef TX433_H_
#define TX433_H_

#include "PrintBuf.h"

class TX433: public PrintBuf {
	int rate;
	int rxData;
	int txData;
	int enable;
	bool pttInvert;

public:
	TX433(const int rate, const int rxData, const int txData, const int enable, bool pttInvert);
	virtual ~TX433();
	void speed(int speed);
	void send();
	void initRhInstance();
};

#endif /* TX433_H_ */
