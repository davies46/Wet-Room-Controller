#include "pins.h"
#include "Constants.h"
#include "sharedVars.h"

class Motors {
private:
	uint16_t prevM1Speed;
	uint16_t prevM2Speed;
	uint16_t softstartCycle;
	int softStart(int requestedSpd, uint16_t &prevSpeed);

public:
	Motors() {
		prevM1Speed = 0;
		prevM2Speed = 0;
		softstartCycle = 0;
	}

	void setSpeed(const uint16_t spd);
};

