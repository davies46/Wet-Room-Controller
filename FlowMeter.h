/*
 * FlowMeter.h
 *
 *  Created on: 3 Sep 2014
 *      Author: pdavies
 */

#ifndef FLOWMETER_H_
#define FLOWMETER_H_


extern void initFlowMeter();

class FlowMeter {
private:
uint16_t cycle;

public:
	FlowMeter();
	~FlowMeter();

	uint64_t timeStoppedInSeconds();
	void initFlowMeter();
	uint64_t deltaTicks();
};
#endif /* FLOWMETER_H_ */
