/*
 * Ticks.h
 *
 *  Created on: 11 Nov 2014
 *      Author: pdavies
 */

#ifndef TICKS_H_
#define TICKS_H_

#define MILLIS_PER_TICK 30
#define MILLIS_PER_TENTH 100
#define MILLIS_PER_SECOND 1000

class Ticks {
private:
	uint64_t firstSecond;
	uint64_t lastSecond;
	uint64_t lastTenth;
	uint64_t lastTick;
	uint64_t thisTick;
	uint64_t thisMillis;
	uint64_t lastClockMillis;

	void setFromMillis() {
		thisSecond = thisMillis / MILLIS_PER_SECOND;
		thisTenth = thisMillis / MILLIS_PER_TENTH;
		thisTick = thisMillis / MILLIS_PER_TICK;
	}

public:
	uint64_t thisSecond;
	uint64_t thisTenth;

	Ticks() {
		lastClockMillis = millis();
		update();
		firstSecond = thisSecond;
		lastSecond = thisSecond;
		lastTenth = thisTenth;
		lastTick = thisTick;
	}
	virtual ~Ticks() {
	}

	void update() {
		uint64_t clockMillis = millis();
		uint64_t delta = clockMillis - lastClockMillis;
		if (clockMillis < lastClockMillis) {
			//it just wrapped
			Serial.println("Clock wrapped");
			thisMillis += 0x0000000100000000ull;
			thisMillis &= 0xFFFFFFFF00000000ull;
			thisMillis |= clockMillis;
		} else {
			thisMillis += delta;
		}
		lastClockMillis = clockMillis;
		setFromMillis();
	}

	uint64_t uptime() {
		return thisSecond - firstSecond;
	}

	bool isNextSecond() {
		if (thisSecond == lastSecond)
			return false;
		lastSecond = thisSecond;
		return true;
	}
	bool isNextTenth() {
		if (thisTenth == lastTenth)
			return false;
		lastTenth = thisTenth;
		return true;
	}
	bool isNextTick() {
		if (thisTick == lastTick)
			return false;
		lastTick = thisTick;
		return true;
	}
	uint64_t getSecond() {
		return thisSecond;
	}
	uint64_t getTenth() {
		return thisTenth;
	}
	uint64_t howLongAgoWas(uint64_t evt) {
		return thisMillis - evt;
	}

	uint64_t ageInSeconds(uint64_t evt) {
		return thisSecond - evt;
	}

	inline uint64_t getMillis() {
		return thisMillis;
	}
};

#endif /* TICKS_H_ */
