/*
 * Extractor.h
 *
 *  Created on: 1 Nov 2014
 *      Author: pdavies
 */

#ifndef EXTRACTOR_H_
#define EXTRACTOR_H_

class Extractor {
private:
	int32_t ticksRemaining;
	long int fanLastSwitchedOffAt;
	long int fanLastSwitchedOnAt;
	bool flashState;

	void stop();
	void setFanStay(uint16_t secs);

public:
	uint16_t humidity;

	Extractor();
	~Extractor() {
	}

	bool isRunning() {
		return ticksRemaining > 0;
	}
	bool inCooldown();

	inline bool atOrAboveThreshold();
	inline bool belowThreshold();
	void start(int d);
	void recordHumidity();
	void checkExtractor();
	void recalibrateToChangeState();
	void actionExtractorFan();
	void tick();
	void *getStayAddress() {
		return &ticksRemaining;
	}
	uint16_t getTicksRemaining() {
		return ticksRemaining;
	}
};

#endif /* EXTRACTOR_H_ */
