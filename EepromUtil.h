/*
 * EepromUtil.h
 *
 *  Created on: Jun 15, 2013
 *      Author: Phil
 */

#ifndef EEPROMUTIL_H_
#define EEPROMUTIL_H_
#include <Arduino.h>
#include "EEPROM.h"
#include "sharedVars.h"
#include "Lights.h"

extern uint16_t getOptionValue(uint8_t optionNum);
extern void setAndSaveOptionValue(uint8_t optionNum, uint16_t val);
extern String getOptionName(uint8_t optionNum);

//extern String optionName[];
extern const int numOptions;

extern String commandName[];
extern const int numCommands;

extern void loadFromEeprom();
extern void saveOption(uint8_t optionNum);
extern void resetEepromToDefaults();
extern void changeOptionBy(uint8_t optionNum, int delta);
extern void listCurrentValues();

template<class T> int EEPROM_write(int ee, const T& value) {
	const byte* p = (const byte*) (const void*) &value;
	uint16_t i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template<class T> int EEPROM_read(int ee, T& value) {
	byte* p = (byte*) (void*) &value;
	uint16_t i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}


#endif /* EEPROMUTIL_H_ */
