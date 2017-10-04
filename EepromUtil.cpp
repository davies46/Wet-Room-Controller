#include "EepromUtil.h"
#include "Constants.h"
#include "Ticks.h"

void saveOption(uint8_t optionNum);

typedef struct {
	String optionName;
	uint16_t optionValue;
	uint16_t optionIndex;
	uint16_t initialOptionValue;
	uint8_t increment;
} Option;

#define RANGE_DIVERTER 0
#define RANGE_BOOLEAN -1

#define VALUE_PLACEHOLDER 0

Option options[] = { { "Trip press", VALUE_PLACEHOLDER, OPT_SWITCHON_PRESSURE, 70, 5 }, //0
		{ "Extractor dwell", VALUE_PLACEHOLDER, OPT_EXTRACTOR_DWELL, 120, 10 }, //1
		{ "Shower dwell", VALUE_PLACEHOLDER, OPT_SHOWER_LIGHTS_STAY, 120 * TICKS_PER_SECOND, 5 }, //2
		{ "Alcove dwell", VALUE_PLACEHOLDER, OPT_ALCOVE_LIGHTS_STAY, 240 * TICKS_PER_SECOND, 5 }, //3
		{ "Danger pressure", VALUE_PLACEHOLDER, OPT_DANGER_PRESSURE_DIF, 300, 5 }, //4
		{ "Humidity trip", VALUE_PLACEHOLDER, OPT_HUMIDITY_THRESHOLD, 600, 5 }, //Keep layout
		{ "Unused", VALUE_PLACEHOLDER, OPT_UNUSED1, 5, 1 }, //Keep layout
		{ "Diverter pos", VALUE_PLACEHOLDER, OPT_DIVERTER_STATE, Diverter::OUTPUT_OVERHEAD, RANGE_DIVERTER }, //Keep layout
		{ "Glitz delay", VALUE_PLACEHOLDER, OPT_GLITZ_DELAY, 300, 1 }, //Keep layout
		{ "Power enable", VALUE_PLACEHOLDER, OPT_POWER_ENABLE, OC_OFF, RANGE_BOOLEAN }, //Keep layout
		{ "DST wrong", VALUE_PLACEHOLDER, OPT_DST_WRONG, false, RANGE_BOOLEAN }, //Keep layout
		{ "Pressure gain", VALUE_PLACEHOLDER, OPT_PRESSURE_GAIN, 50, 5 }, //Keep layout
		{ "Flow gain", VALUE_PLACEHOLDER, OPT_FLOW_GAIN, 25, 1 }, //Keep layout
		{ "Humid cal", VALUE_PLACEHOLDER, OPT_HUMIDITY_CAL, 15, 1 }, //Keep layout
		{ "Cold temp", VALUE_PLACEHOLDER, OPT_COLD_TEMP, 200, 1 }, //Keep layout
		{ "Purge delay", VALUE_PLACEHOLDER, OPT_PURGE_DELAY, 50, 1 }, //Keep layout
		{ "Purge time", VALUE_PLACEHOLDER, OPT_PURGE_TIME, 5, 1 }, //Keep layout
		{ "Start visual", VALUE_PLACEHOLDER, OPT_STARTUP_VISUAL, 6, 1 }, //Keep layout
		{ "Flow trip", VALUE_PLACEHOLDER, OPT_FLOW_TRIP, 5, 1 }, //Keep layout
		{ "Flow power", VALUE_PLACEHOLDER, OPT_FLOW_POWERPUMP_TRIP, 15, 1 }, //Keep layout
		{ "Magfld gain", VALUE_PLACEHOLDER, OPT_MAGFIELD_GAIN, 10, 1 }, //Keep layout
		{ "Danger Magfld", VALUE_PLACEHOLDER, OPT_MAGFIELD_DANGER, 224, 1 }, //Keep layout
		{ "Trip Magfld", VALUE_PLACEHOLDER, OPT_MAGFIELD_TRIP, 60, 5 }, //Keep layout
		{ "Leak rate", VALUE_PLACEHOLDER, OPT_PUMP_LEAKY_BUCKET_LEAK_RATE, 10, 1 }, //Keep layout
		{ "Mag Calib Ofs", VALUE_PLACEHOLDER, OPT_MAG_CAL_OFS, 0, 1 }, //Keep layout
		{ "Time Full", VALUE_PLACEHOLDER, OPT_MAXTIME_FULL_DRAIN, 3, 1 }, //4
		{ "MAX Extractor", VALUE_PLACEHOLDER, OPT_MAXTIME_EXTRACTOR, 30, 2 }, //4
		};

uint8_t diverterValues[] = { Diverter::OUTPUT_OVERHEAD, Diverter::OUTPUT_BOTH_W, Diverter::OUTPUT_WAND, Diverter::OUTPUT_BOTH_O };
uint8_t booleanValues[] = { 0, 1 };

uint8_t *validValues[] = { diverterValues, booleanValues };
uint8_t enumSize[] = { ARRAY_ELEMENTS(diverterValues), ARRAY_ELEMENTS(booleanValues) };

const int numOptions = ARRAY_ELEMENTS(options);
//uint16_t optionValue[ARRAY_ELEMENTS(options)];

String commandName[] = { "Reset options", "Cycle divertor" };
const int numCommands = sizeof(commandName) / sizeof(String);

uint16_t getOptionValue(uint8_t optionNum) {
	return options[optionNum].optionValue;
}

void setAndSaveOptionValue(uint8_t optionNum, uint16_t val) {
	options[optionNum].optionValue = val;
	saveOption(optionNum);
}

String getOptionName(uint8_t optionNum) {
	return options[optionNum].optionName;
}

void readOrUpdateEeprom(uint16_t adr, uint16_t &val, uint16_t deflt) {
	EEPROM_read(adr, val);
	if (val == 0xFFFF) {
		val = deflt;
		EEPROM_write(adr, val);
	}
}

void resetEepromToDefaults() {
	for (int o = 0; o < numOptions; o++) {
		options[o].optionValue = options[o].initialOptionValue;
		Serial.print("Reset ");
		Serial.print(options[o].optionName);
		Serial.print(" to ");
		Serial.println(options[o].optionValue);
		EEPROM_write(options[o].optionIndex * 2, options[o].optionValue);
	}
}

void listCurrentValues() {
	for (int o = 0; o < numOptions; o++) {
		Serial.print(o);
		Serial.print(": ");
		Serial.print(options[o].optionName);
		Serial.print(": ");
		Serial.println(options[o].optionValue);
	}
}

void loadFromEeprom() {
	for (int o = 0; o < numOptions; o++) {
		readOrUpdateEeprom(options[o].optionIndex * 2, options[o].optionValue, options[o].initialOptionValue);
		Serial.print("Load: ");
		Serial.print(options[o].optionName);
		Serial.print(", value: ");
		Serial.println(options[o].optionValue);
	}
}

void saveOption(uint8_t optionNum) {
	EEPROM_write(options[optionNum].optionIndex * 2, options[optionNum].optionValue);
}

void changeOptionBy(uint8_t optionNum, int delta) {
	int incr = options[optionNum].increment;
	if (incr > 0) {
		options[optionNum].optionValue += options[optionNum].increment * delta;
	} else {
		uint16_t currentValue = options[optionNum].optionValue;
		uint8_t *values = validValues[-incr];
		uint8_t enums = enumSize[-incr];
		for (int currentValueIdx = 0; currentValueIdx < enums; currentValueIdx++) {
			if (values[currentValueIdx] == currentValue) {
				int newValIdx = (currentValueIdx + delta + 10 * enums) % enums;
				options[optionNum].optionValue = values[newValIdx];
				break;
			}
		}
	}
}
