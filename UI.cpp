/*
 * OLED.cpp
 *
 *  Created on: 13 Sep 2014
 *      Author: pdavies
 */

#include "Adafruit_SSD1306.h"
#include "Constants.h"
#include "EepromUtil.h"
#include "Diverter.h"
#include "DHT22.h"
#include "RTClib.h"
#include "UI.h"

#define OLED_RESET 4

//static const uint8_t PROGMEM logo16_glcd_bmp[] = { B00000000, B11000000, B00000001, B11000000, B00000001, B11000000, B00000011,
//		B11100000, B11110011, B11100000, B11111110, B11111000, B01111110, B11111111, B00110011, B10011111, B00011111, B11111100,
//		B00001101, B01110000, B00011011, B10100000, B00111111, B11100000, B00111111, B11110000, B01111100, B11110000, B01110000,
//		B01110000, B00000000, B00110000 };
DisplayNS::DisplayMode nextDisplayMode[] = { DisplayNS::OPTIONS, DisplayNS::VALUES, DisplayNS::COMMAND, DisplayNS::NORMAL };

//#define OPT_SWITCHON_PRESSURE 0
//#define OPT_EXTRACTOR_DWELL 1
//#define OPT_SHOWER_DWELL 2
//#define OPT_ALCOVE_LIGHTS_STAY 3
//#define OPT_DANGER_PRESSURE_DIF 4
//#define OPT_HUMIDITY_THRESHOLD 5
//#define OPT_SHOWER_FLOW_MULTIPLIER 6
unsigned long dialTime, lastDialTime;

String valueNames[] = { "Mean pDif", "pDif", "Spur press", "Max pDif", "Panic", "Flt A", "Flt B", "Pmp Bkt", "Alc tcks", "SLt tks",
		"Ext fn", "Last cal", "Last act", "Max flow", "Humidity", "DHT cnv", "Second", "Time2panic", "dial cnt", "DHT erct",
		"DHT lst" };

String dayName[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };

uint16_t dummy;
void *valuePointers[] = { &meanPressureDifferential,
#if PRESSURE_SENSOR
		&pressure.instantaneousPressureDifferential,
#else
		&dummy,
#endif
		&spuriousPressureDifferentialCount, &maxAbsSpuriousPressureDif, &panicking, &floatA, &floatB, &pump.leakyBucket,
		&lights.ticksToAlcoveLightsOut, &lights.ticksToShowerLightsOut, extractor.getStayAddress(), &tMinLastCalibration,
		&tMinLastActivity, &maxDeltaFlowPulses, &extractor.humidity, &numDhtConversions, &ticks.thisSecond, &timeToPanic, &dialCnt,
		&dhtNumErrors, &dhtLastError };

uint8_t valueSize[] = { FLT_T, FLT_T, UINT32_T, UINT32_T, BOOL_T, BOOL_T, BOOL_T, UINT16_T, UINT16_T, UINT16_T, UINT16_T, UINT32_T,
		UINT32_T, UINT16_T, UINT16_T, UINT16_T, UINT16_T, UINT16_T, UINT32_T, UINT16_T, UINT8_T };

UI::UI(String statusLine) {
	if (rtc->begin()) {
//	rtc->adjust(DateTime(__DATE__, __TIME__));
		if (!rtc->isrunning()) {
			Serial.println("RTC is NOT running!");
			// following line sets the RTC to the date & time this sketch was compiled
//			rtc->adjust(DateTime(__DATE__, __TIME__));
		}
	}
	if (ARRAY_ELEMENTS(valueNames) != ARRAY_ELEMENTS(valuePointers) || ARRAY_ELEMENTS(valueNames) != ARRAY_ELEMENTS(valueSize)) {
		Serial.println("Wrong element count on value pointers array etc.");
		Serial.println(ARRAY_ELEMENTS(valueNames));
		Serial.println(ARRAY_ELEMENTS(valuePointers));
		Serial.println(ARRAY_ELEMENTS(valueSize));
	}
	oledDisplay = new Adafruit_SSD1306();
	oledDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3D (for the 128x64)
	oledDisplay->display();
	currentOptionNum = OPT_UNUSED1;
	currentCommandNum = currentValueNum = 0;
	editing = false;
	lastDialTime = ticks.getMillis();
	timeLastDisplayModeEntered = lastDialTime;
	this->statusLine = statusLine;

	setDisplayMode(INITIAL_DISPLAY_MODE);
	showCalibMsg();
}

void resetOptions() {
	maxDeltaFlowPulses = 0;
}

UI::~UI() {
	// TODO Auto-generated destructor stub
}

void UI::showCalibMsg() {
	oledDisplay->clearDisplay();
	showStatus();
	oledDisplay->setTextSize(4);
	oledDisplay->setCursor(0, 20);
	oledDisplay->setTextColor(WHITE);
	oledDisplay->print("Calib");
	oledDisplay->display();
}

void UI::update() {
	//revert to normal display if no action on other modes for a while
	if ((ticks.getMillis() - timeLastDisplayModeEntered) > 10000) {
		displayMode = DisplayNS::NORMAL;
	}

	oledDisplay->fillRect(0, 0, 128, 64, BLACK);

	if (rtc->isrunning()) {
		timeNow = rtc->now();
	} else {
		timeNow = DateTime(2014, 9, 5, 18, 45, 23);
	}

	hour = timeNow.hour();
	min = timeNow.minute();
	month = timeNow.month();
	day = timeNow.day();
	dow = timeNow.dayOfWk();
	year = timeNow.year();
	sec = timeNow.second();

	showStatus();

	switch (displayMode) {
	case DisplayNS::NORMAL:
		updateDisplayNormal();
		break;

	case DisplayNS::OPTIONS:
		// Always make the central option the current one
		oledDisplay->setTextSize(1);

		for (uint8_t l = 0; l < 5; l++) {
			setCursor(0, l);
			inverse(l == 2 && !editing);
			uint8_t optionNum = (l - 2 + numOptions + currentOptionNum) % numOptions;
			oledDisplay->print(getOptionName(optionNum));
			inverse(l == 2 && editing);
			setCursor(17, l);
			oledDisplay->print(getOptionValue(optionNum));
		}
		oledDisplay->display();
		break;

	case DisplayNS::VALUES:
		// Always make the central option the current one
		oledDisplay->setTextSize(1);

		for (uint8_t l = 0; l < 5; l++) {
			setCursor(0, l);
			inverse(l == 2);
			uint8_t valueNum = (l - 2 + ARRAY_ELEMENTS(valueNames) + currentValueNum) % ARRAY_ELEMENTS(valueNames);
			oledDisplay->print(valueNames[valueNum]);
			setCursor(14, l);

			uint8_t v;
			switch (valueSize[valueNum]) {

			case UINT32_T:
				oledDisplay->print(*((uint32_t *) valuePointers[valueNum]));
				break;

			case UINT16_T:
				oledDisplay->print(*((uint16_t *) valuePointers[valueNum]));
				break;

			case UINT8_T:
				oledDisplay->print(*((uint8_t *) valuePointers[valueNum]));
				break;

			case FLT_T:
				oledDisplay->print(*((float *) valuePointers[valueNum]));
				break;

			case BOOL_T:
				v = *((uint8_t *) valuePointers[valueNum]);
				oledDisplay->print((v == 0) ? 0 : 1);
				break;

			default:
				break;
			}
		}
		oledDisplay->display();
		break;

	case DisplayNS::COMMAND:
		// Always make the central option the current one
		oledDisplay->setTextSize(1);

		for (uint8_t l = 0; l < 5; l++) {
			setCursor(0, l);
			inverse(l == 2);
			uint8_t commandNum = (l - 2 + numCommands + currentCommandNum) % numCommands;
//			Serial.print(commandNum);
			oledDisplay->print(commandName[commandNum]);
		}
//		Serial.println();
		oledDisplay->display();
		break;

	}
}

void UI::updateDisplayNormal() {
	setCursor(0, 0);
	oledDisplay->print("Spu:");
	if (sec & 1) {
		oledDisplay->print(spuriousPressureDifferentialCount);
	} else {
		uint32_t pDif = maxAbsSpuriousPressureDif;
		uint16_t power = 0;
		while (pDif > 0) {
			power++;
			pDif /= 10;
		}
		oledDisplay->print("10^");
		oledDisplay->print(power);
	}
	setCursor(10, 0);
	if (sec & 1) {
		oledDisplay->print("Hum:");
		oledDisplay->print(extractor.humidity * 0.1, 1);
		oledDisplay->print("   ");
	} else {
		oledDisplay->print("Cvs:");
		oledDisplay->print(getDhtStateSymbol());
		oledDisplay->print(" ");
		oledDisplay->print(numDhtConversions);
		oledDisplay->print("   ");
	}
	setCursor(0, 2);
	oledDisplay->print("ShLt:");
	oledDisplay->print(lights.ticksToShowerLightsOut);
	oledDisplay->print("   ");

	setCursor(8, 2);
	oledDisplay->print("   Acv:");
	oledDisplay->print(lights.ticksToAlcoveLightsOut);
	oledDisplay->print("   ");

	setCursor(0, 1);
	oledDisplay->print("Gulp:");
	oledDisplay->print(pump.leakyBucket);
	oledDisplay->print("   ");

	setCursor(8, 1);
	oledDisplay->print("   Fan:");
	oledDisplay->print(extractor.getTicksRemaining());
	oledDisplay->print("   ");
#define _GRAPHIC_PRESSURE
#ifdef _GRAPHIC_PRESSURE
//		showInstantaneousPressure();
#else
	setCursor(0, 3);
	oledDisplay->print("Inst pDiff: ");
	oledDisplay->print(instantaneousPressureDifferential);
#endif
	setCursor(0, -1);
	inverse(pump.leakyBucket);
	oledDisplay->print('P');

	setCursor(2, -1);
	inverse(lights.ticksToShowerLightsOut);
	oledDisplay->print('S');

	setCursor(4, -1);
	inverse(lights.ticksToAlcoveLightsOut);
	oledDisplay->print('A');

	setCursor(6, -1);
	inverse(extractor.getTicksRemaining());
	oledDisplay->print('X');

	setCursor(8, -1);
	inverse(panicking);
	oledDisplay->print('E');

	setCursor(15, -1);
	inverse(floatA);
	oledDisplay->print('A');
	inverse(floatB);
	oledDisplay->print('B');

	setCursor(18, -1);
	inverse((getOptionValue(OPT_DIVERTER_STATE) & Diverter::OUTPUT_OVERHEAD) != 0);
	oledDisplay->print("O");

	inverse((getOptionValue(OPT_DIVERTER_STATE) & Diverter::OUTPUT_WAND) != 0);
	oledDisplay->print("W");

	inverse(false);

	setCursor(0, 4);

	if (timeSinceLastOk > 2) {
		if (timeToPanic < 0) {
			oledDisplay->print(" Disable!");
		} else {
			oledDisplay->print("   ");
			print2digits(timeSinceLastOk);
			oledDisplay->print('!');
			oledDisplay->print("   ");
		}
	} else {
		print2digits(hour);
		oledDisplay->print(':');
		print2digits(min);
		oledDisplay->print(':');
		print2digits(sec);

		oledDisplay->print("  ");
		oledDisplay->print(dayName[dow]);
		oledDisplay->print('/');
		print2digits(day);
		oledDisplay->print('/');
		print2digits(month);
		oledDisplay->print('/');
		print2digits(year % 100);
	}
}

void UI::updateDisplayCyclic() {
	setCursor(0, 0);
	oledDisplay->setTextSize(2);
	switch (sec) {

	case 0:
		oledDisplay->println("Humidity");
		oledDisplay->print(extractor.humidity * 0.1, 1);
		break;
	case 1:
		oledDisplay->println("Gulpers");
		oledDisplay->print(pump.leakyBucket);
		break;
	case 2:
		oledDisplay->println("Mag compass");
		oledDisplay->print(adjustedMagFld);
		break;

	default:
		updateDisplayNormal();
		break;
	}
}

#define PRESSURE_LINE_CENTRE 47
void UI::showInstantaneousLevel() {
	if (displayMode == DisplayNS::NORMAL) {
		oledDisplay->drawLine(0, 46, 127, 46, BLACK);
		int v;
#if PRESSURE_SENSOR
		v = (int) pressure.instantaneousPressureDifferential;
		oledDisplay->drawLine(64, PRESSURE_LINE_CENTRE - 2, 64, PRESSURE_LINE_CENTRE + 2, WHITE);
		if (v > 0) {
			v = min(v,63);
			oledDisplay->drawLine(64, PRESSURE_LINE_CENTRE - 1, 64 + v, PRESSURE_LINE_CENTRE - 1, WHITE);
		} else {
			v = min(-v,63);
			oledDisplay->drawLine(64, PRESSURE_LINE_CENTRE - 1, 64 - v, PRESSURE_LINE_CENTRE - 1, WHITE);
		}
#endif
		v = adjustedMagFld / 2;
		if (v > 0) {
			v = min(v,63);
			oledDisplay->drawLine(64, PRESSURE_LINE_CENTRE + 1, 64 + v, PRESSURE_LINE_CENTRE + 1, WHITE);
		} else {
			v = min(-v,63);
			oledDisplay->drawLine(64, PRESSURE_LINE_CENTRE + 1, 64 - v, PRESSURE_LINE_CENTRE + 1, WHITE);
		}
		oledDisplay->display();
	}
}

void UI::adjustCurrentOptionValue(bool fwd) {
	dialTime = ticks.getMillis();
	unsigned long delta = dialTime - lastDialTime;
	lastDialTime = dialTime;
	Serial.print(getOptionValue(currentOptionNum));
	Serial.print("->");
	int change = (delta < 5) ? 10 : (delta < 15) ? 5 : 1;
	changeOptionBy(currentOptionNum, fwd ? change : -change);
	Serial.println(getOptionValue(currentOptionNum));

	if (displayMode == DisplayNS::NORMAL) {
		inverse(false);
		oledDisplay->fillRect(78, 8, 24, 9, BLACK);
		setCursor(11, -1);
		oledDisplay->print(getOptionValue(currentOptionNum));

		Serial.print("Adjusted option ");
		Serial.print(getOptionName(currentOptionNum));
		Serial.print(" to ");
		Serial.println(getOptionValue(currentOptionNum));
	}
}

void UI::dial(bool fwd) {
	switch (displayMode) {
	case DisplayNS::NORMAL:
		adjustCurrentOptionValue(fwd);
		break;

	case DisplayNS::OPTIONS:
		if (editing) {
			adjustCurrentOptionValue(fwd);
		} else {
			if (fwd)
				currentOptionNum++;
			else
				currentOptionNum--;
			currentOptionNum = (numOptions + currentOptionNum) % numOptions;
		}
		break;

	case DisplayNS::COMMAND:
		if (fwd)
			currentCommandNum++;
		else
			currentCommandNum--;
		currentCommandNum = (numCommands + currentCommandNum) % numCommands;

		break;

	case DisplayNS::VALUES:
		if (fwd)
			currentValueNum++;
		else
			currentValueNum--;
		currentValueNum = (ARRAY_ELEMENTS(valueNames) + currentValueNum) % ARRAY_ELEMENTS(valueNames);
		break;
	}

	if (displayMode != DisplayNS::NORMAL) {
		timeLastDisplayModeEntered = ticks.getMillis();
	}
	oledDisplay->display();
}

void UI::shortPress() {
	switch (displayMode) {
	case DisplayNS::OPTIONS:
		if (editing) {
			saveOption(currentOptionNum);
		}
		editing = !editing;
		timeLastDisplayModeEntered = ticks.getMillis();
		break;

	case DisplayNS::COMMAND:
		timeLastDisplayModeEntered = ticks.getMillis();
		switch (currentCommandNum) {
		case DisplayNS::RESET_OPTIONS:
			resetOptions();
			resetEepromToDefaults();
			break;

		case DisplayNS::CYCLE_DIVERTER:
			diverter->cycle();
			break;
		}
		break;

	default:
		break;
	}
	update();
}

void UI::inverse(bool iv) {
	if (iv)
		oledDisplay->setTextColor(BLACK, WHITE);
	else
		oledDisplay->setTextColor(WHITE);
}

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

void UI::setCursor(int col, int row) {
	oledDisplay->setCursor(6 * col, 17 + 9 * row);
}

void UI::cycleDisplayMode() {
	displayMode = nextDisplayMode[displayMode];
	timeLastDisplayModeEntered = ticks.getMillis();
}

void UI::setDisplayMode(DisplayNS::DisplayMode mode) {
	displayMode = mode;
}

void UI::print2digits(int v) {
	if (v < 10) {
		oledDisplay->print('0');
	}
	oledDisplay->print(v);
}

void UI::showStatus() {
	oledDisplay->setCursor(0, 0);
	oledDisplay->setTextSize(1);
	oledDisplay->print(statusLine);
}

void UI::testfillroundrect(void) {
	uint8_t color = WHITE;
	for (int16_t i = 0; i < oledDisplay->height() / 2 - 2; i += 2) {
		oledDisplay->fillRoundRect(i, i, oledDisplay->width() - 2 * i, oledDisplay->height() - 2 * i, oledDisplay->height() / 4,
				color);
		if (color == WHITE)
			color = BLACK;
		else
			color = WHITE;
		oledDisplay->display();
	}
}
