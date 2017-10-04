/*
 * OLED.h
 *
 *  Created on: 13 Sep 2014
 *      Author: pdavies
 */

#ifndef OLED_H_
#define OLED_H_

#include <Arduino.h>
#include "Namespace.h"
#include "sharedVars.h"
#include "RTClib.h"
#include "Pressure.h"
#include "Adafruit_SSD1306.h"
#include "Constants.h"

extern DisplayNS::DisplayMode nextDisplayMode[];

class UI {
private:
	float desiredTemp;
	float actualTemp;
//	RTC_DS1307 *rtc;
//	Pressure *pressure;
	DateTime timeNow;
	DisplayNS::DisplayMode displayMode;
//	DisplayNS::DisplayMode mode;
	int dutyCycle;
	uint64_t seconds;
	byte header;
	float pid;
	uint64_t timeLastDisplayModeEntered;
	String statusLine;
	Adafruit_SSD1306 *oledDisplay;
	uint16_t currentOptionNum, currentCommandNum, currentValueNum;
	bool editing;
	void testfillroundrect();
	void setCursor(int col, int row);
	void testdrawbitmap(uint8_t w, uint8_t h);
	void inverse(bool iv);
	void internalDial();
	void adjustCurrentOptionValue(bool fwd);

	void updateDisplayNormal();
	void updateDisplayCyclic();
	int hour;
	int min;
	int month;
	int day;
	int dow;
	int year;
	int sec;

public:
	UI(String statusLine);
	virtual ~UI();

	void showStatus();
	void setDisplayMode(DisplayNS::DisplayMode mode);
	void cycleDisplayMode();
	void update();
	void showInstantaneousLevel();
	void dial(bool fwd);
	void print2digits(int v);
	void showCalibMsg();
	void shortPress();
};

#endif /* OLED_H_ */
