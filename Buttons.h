/*
 * Buttons.h
 *
 *  Created on: 19 Nov 2014
 *      Author: pdavies
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

class Buttons {
private:
	bool divertorBtnState;
	bool extractorBtnState;
	uint64_t timeExtractorButtonWasFirstPressed;
	uint64_t lastDiverterBtnAction;

public:
	Buttons();
	virtual ~Buttons();

	void check();
};

#endif /* BUTTONS_H_ */
