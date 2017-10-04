/*
 * Interrupts.h
 *
 *  Created on: 19 Nov 2014
 *      Author: pdavies
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <Arduino.h>

class Interrupts {
public:
	Interrupts();
	virtual ~Interrupts();
	void resetInterruptCounts();
	void tick();
};

#endif /* INTERRUPTS_H_ */
