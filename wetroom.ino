#include <Arduino.h>
#include <avr/wdt.h>
#include "Adafruit_BMP085_U.h"
#include "Buffer.h"
#include "Message.h"
#include "pins.h"
#include "motors.h"
#include "timerUtil.h"
#include "Timer4.h"
#include "DHT22.h"
#include "SamplesBuffer.h"
#include "Pressure.h"
#include "sharedVars.h"
#include "FlowMeter.h"
#include "Constants.h"
#include "UI.h"
#include "EepromUtil.h"
#include "Diverter.h"
#include "Buttons.h"
#include "Extractor.h"
#include "Ticks.h"
#include "Lights.h"
#include "magnetometer.h"
#include "Interrupts.h"

#define WIRELESS 0
#if WIRELESS
#include "TX433.h"
#define LOG(s) {Serial.print(s);if (broadcast){tx433->print(s);tx433->send();}}
#else
#define LOG(s) {Serial.print(s);}
#endif

/*
 * Assume hardware integrity and update software if there's a hardware failure!
 */

#define _USING_DHT
#define _TIMER4

int m1Lvl = 0;

Ticks ticks;
uint64_t lastTimeWaterDetected;
uint64_t timeLeveLastOk;

bool panicking;
bool dhtOk;
uint32_t eventsLogged;

uint32_t flowPulsesThisSecond;
bool floatA;
bool floatB;
bool panickingEnable;
int timeToPanic;
int timeSinceLastOk;
bool calibratedSinceLastPump;
byte lightBlinker;
bool startupCommplete;
bool dangerLevel;
int16_t maxDeltaFlowPulses;

uint64_t tMinLastActivity;
uint64_t tMinLastCalibration;
long int adjustedMagFld;
long int magfldLow = 0, magfldHigh = 0;

uint8_t led = 0;

String CMD_PUMP_KICK = "p - kick pumps into life";
String CMD_LEAK_RATE = "l - pump bucket leak rate";
String CMD_EXTRACTOR_KICK = "x - kick extractors into life";
String CMD_FAKE_PRESSURE = "fnnn - fake pressure dif reading to nnn";
String CMD_CYCLE_DISPLAYMODE = "d - cycle display mode";
String CMD_RESYNC = "s - sync pressure sensors";
String CMD_RESET = "r - reset some counters";
String CMD_DIVERT = "c - cycle diverter";
String CMD_SET_MAG_TRIP = "m - mag fld trip";
String CMD_VIEW_SETTINGS = "v - view settings";
String CMD_UPDATE_UPTION = "o - update option o v";

float meanPressureDifferential;

Interrupts interrupts;
Buttons buttons;
Diverter *diverter;
Lights lights = Lights();
#if USE_PRESSURE
Pressure pressure;
#endif

RTC_DS1307 *rtc;
FlowMeter *flowMeter;
UI *oled;
Extractor extractor;
Pump pump = Pump();
Motors motors = Motors();
ButtonLights buttonLights = ButtonLights();
Magnetometer *magnetometer;
#if WIRELESS
TX433 *tx433;
#endif

#define countdownTick(v) if (v > 0) v -= 1; if (v < 0) v = 0

void setup() {
	Serial.begin(115200);
	log("Hello");
	eventsLogged = 0;
	ticks = Ticks();
	lastTimeWaterDetected = 0;
	extractor = Extractor();
	lightBlinker = 0;
	maxDeltaFlowPulses = 0;
	calibratedSinceLastPump = false;
	startupCommplete = false;
	dhtOk = false;
	log("Load settings from Eeprom...");

	loadFromEeprom();
	log("...loaded, set outputs and pin modes");

	Output(PIN_LED, ON);

	Output(PIN_MOTOR1_PWM, OFF);
	Output(PIN_MOTOR2_PWM, OFF);

	Output(PIN_EXTRACTOR_FAN, OC_OFF);

	Output(PIN_RL1, RL_OFF);
	Output(PIN_RL2, RL_OFF);
	Output(PIN_RL3, RL_OFF);
	Output(PIN_RL4, RL_OFF);
	Output(PIN_RL5, RL_OFF);
	Output(PIN_RL6, RL_OFF);
	Output(PIN_RL7, RL_OFF);
	Output(PIN_RL8, RL_OFF);

	Output(PIN_POWERSHOWER_PUMP_ENABLE, OC_OFF);

	//initialising light

	Output(PIN_SHOWERLIGHTS_PWM, OFF);

	Output(PIN_BMP_XCLR_AIR, OFF);
	Output(PIN_BMP_XCLR_WATER, OFF);

	Output(PIN_OUTPUT_OVERHEAD_A, ON);
	Output(PIN_OUTPUT_OVERHEAD_B, OFF);

	Output(PIN_OUTPUT_WAND_A, OFF);
	Output(PIN_OUTPUT_WAND_A, ON);

	pinMode(PIN_WATER_LEVEL_A, INPUT_PULLUP);
	pinMode(PIN_WATER_LEVEL_B, INPUT_PULLUP);

	pinMode(PIN_BUTTON_LIGHTS, INPUT_PULLUP);
	pinMode(PIN_BUTTON_EXTRACTOR, INPUT_PULLUP);
	pinMode(PIN_BUTTON_PUMP, INPUT_PULLUP);
	pinMode(PIN_SHOWER_FLOW_METER, INPUT_PULLUP);
	pinMode(PIN_BTN_DIAL, INPUT_PULLUP);
	pinMode(PIN_BUTTON_DIVERTER, INPUT_PULLUP);

	pinMode(PIN_ANA_HUMIDITY1, INPUT_PULLUP);
	pinMode(PIN_ANA_HUMIDITY2, INPUT_PULLUP);

	pinMode(PIN_DIAL_A, INPUT_PULLUP);
	pinMode(PIN_DIAL_B, INPUT_PULLUP);
	log("...done, set pwm freqs");

	setPwmFrequency(PIN_MOTOR1_PWM, 1);
	setPwmFrequency(PIN_MOTOR2_PWM, 1);
	log("...done, set motor speed");

#if WIRELESS
	tx433 = new TX433(3000, PIN_UNUSED, PIN_433_TXDATA, PIN_433_TXENABLE, 0);
#endif
	motors.setSpeed(0);

	timeLeveLastOk = ticks.getSecond();
	panicking = false;
	log("...done, init I2C");

	Wire.begin();

#if MAGNETOMETER
	log("...done, Calib magnet");
	magnetometer = new Magnetometer(12345);
	log("...done, show details");
	/* Display some basic information on this sensor */
	magnetometer->displaySensorDetails();
#endif

	log("...done, init RTC");

	rtc = new RTC_DS1307();
	rtc->begin();
	log("...done, init flow meter");

	flowMeter = new FlowMeter();

#if USE_PRESSURE
	pressure = Pressure();
	pressure.init();
#endif

	log("Oled init...");
	const char *date = __DATE__;
	const char *time = __TIME__;
	const char *rev = "$Rev: 726 $";
	char header[32];
	memset(header, ' ', 32);
	char *bufP = header;
	strncpy(bufP, date, 7);
	bufP += 7;
	strncpy(bufP, time, 5);
	bufP += 6;
	strncpy(bufP, rev + 1, 9);
	bufP += 9;
	*bufP = '\0';
	oled = new UI(header);
	log("Oled inited");
	/* Initialise the sensor */

	if (getOptionValue(OPT_DST_WRONG)) {
		// rtc->adjust(DateTime(__DATE__, __TIME__));
		DateTime now = rtc->now();
		rtc->adjust(DateTime(now.year(), now.month(), now.day(), now.hour() - 1, now.minute(), now.second()));
		setAndSaveOptionValue(OPT_DST_WRONG, false);
	}
	diverter = new Diverter();

	interrupts = Interrupts();

	buttonLights.startCycling();

	wdt_enable(WDTO_8S);
	log("Initialized");

}

void loop() {
	led = 1 - led;
	digitalWrite(PIN_LED, led);
	handleEvents();
	handleCommand();

	ticks.update();

	if (ticks.isNextTick())
		doPerTickActions();

	if (ticks.isNextTenth()) {
		doPerSecondActions(ticks.thisTenth % 10);
		doPerTenthSecondActions();
	}
}

void log(const char msg[]) {
	Serial.println(msg);
	delay(100);
}

void doPerTenthSecondActions() {
	/*
	 * Set pump and its light
	 */
	motors.setSpeed(pump.getEffectiveMotorSpeed());
	buttonLights.buttonLight(BL_PUMP, pump.requested());

	if (ticks.getTenth() % 7 == 0)
		lightBlinker = ++lightBlinker % 5;

	extractor.actionExtractorFan();
}

void doPerTickActions() {
	buttons.check();
	interrupts.tick();
}

void doPerSecondActions(int tenthsDigit) {
	lights.tick();

	switch (tenthsDigit) {
	case 0:
		dangerLevel = false;
		wdt_reset();
		break;

	case 1:
		/*
		 * Things that happen every second
		 */
		checkCalendar();
		break;

	case 2:
		checkStartHumidityMeasurement();
		break;

	case 3:
		magnetometerActions();
		break;

	case 4:
		checkStartupComplete();
		extractor.tick();
		break;

	case 5:
		checkPressureSensor();
		break;

	case 6:
		checkFlowSensor();
		break;

	case 7:
		break;

	case 8:
		checkPanic();
		diverter->tick();
		pump.tick();
		break;

	case 9:
		extractor.tick();
		checkResyncSensorsOnIdle();

		detailedLogging();
		oled->update();
		oled->showInstantaneousLevel();
		if (!dangerLevel) {
			timeLeveLastOk = ticks.getSecond();
		}

		//every hour
		if (ticks.getSecond() % 3600 == 0) {
			//check to see if we need to put the clock back or forward
			rtc->checkDst();
		}
		break;

	default:
		break;
	}

}

void checkStartupComplete() {
//Check if startup period is up
	if (!startupCommplete && ticks.uptime() >= getOptionValue(OPT_STARTUP_VISUAL)) {
		startupCommplete = true;
		buttonLights.stopCycling();
		//make sure button lights are displaying what they should be now.
		buttonLights.buttonLight(BL_RESET, OC_ON);
		diverter->restoreFromEeprom();
	}

	//if the pump's been off for N seconds, recalibrate
//	if (pump.doOffActions(300)) {
//		magnetometer->calibrate();
//	}
}

void checkPressureSensor() {
#if USE_PRESSURE
	meanPressureDifferential = pressure.measurePressureDifferential();
	if (meanPressureDifferential > getOptionValue(OPT_SWITCHON_PRESSURE)) {
		uint32_t excessPressure = (uint16_t) meanPressureDifferential - getOptionValue(OPT_SWITCHON_PRESSURE);
		Serial.print("pDif sw");
//		pump.requestMinSpeed(excessPressure * getOptionValue(OPT_PRESSURE_GAIN) / 100);
//		if (meanPressureDifferential > getOptionValue(OPT_DANGER_PRESSURE_DIF)) {
//			dangerLevel = true;
//		}
	}
#endif
}

void checkFlowSensor() {
#if FLOW_SENSOR
	flowPulsesThisSecond = flowMeter->deltaTicks();
	//full speed is ~533 ml/sec or ~239 pulses, ~20 pulses per sec
	//		millilitresPerSecond = pulses * 1000 / PULSES_PER_LITRE;
	maxDeltaFlowPulses = max(maxDeltaFlowPulses, flowPulsesThisSecond);
	if (flowPulsesThisSecond > getOptionValue(OPT_FLOW_TRIP)) {
		Serial.print("flw sw");
		if (flowPulsesThisSecond > getOptionValue(OPT_FLOW_POWERPUMP_TRIP) && getOptionValue(OPT_POWER_ENABLE)) {
			digitalWrite(PIN_POWERSHOWER_PUMP_ENABLE, OC_ON);
		} else {
			digitalWrite(PIN_POWERSHOWER_PUMP_ENABLE, OC_OFF);
		}
		uint16_t minPumpSpeedRequestedByFlow = flowPulsesThisSecond * getOptionValue(OPT_FLOW_GAIN);
		diverter->primeForPurge();
		if (minPumpSpeedRequestedByFlow > 0) {
#if PUMP_ON_FLOW
			pump.requestMinSpeed(minPumpSpeedRequestedByFlow);
#endif

			lights.switchAllOn();
			if (dhtGetTemp10() > getOptionValue(OPT_COLD_TEMP)) {
				extractor.start(3);
			}
		}
	}
#endif
}

void checkStartHumidityMeasurement() {
	/*
	 * Other stuff
	 */
	if (ticks.getSecond() % 8 == 0) {
		// every few seconds initiate a humidity measurement, but not if the shower's running and it's cold :)
		bool warm = dhtGetTemp10() >= getOptionValue(OPT_COLD_TEMP);
		bool showerStopped = flowPulsesThisSecond == 0;
		if (warm || showerStopped) {
			extractor.checkExtractor();
		}
#ifdef _USING_DHT
		startDHT();
#endif
	}
}

void checkResyncSensorsOnIdle() {
	/*
	 * 10 minutes after all activity ceased, re-sync pressure sensors
	 */
	if (!pump.requested() && flowPulsesThisSecond == 0) {
		if (pump.beenOffForSeconds() >= 600 && !calibratedSinceLastPump) {
#if USE_PRESSURE
			pressure.calibratePressureSensors();
#endif
			// magnetometer->calibrate();   // not sure this is really worth it
			calibratedSinceLastPump = true;
		}
	}
}

void magnetometerActions() {
#if MAGNETOMETER
	if (magnetometer->isPresent()) {
		bool ok = magnetometer->read();
		if (!ok) {
			Serial.println("Mag read FAIL");
		} else {
			adjustedMagFld = magnetometer->getX() - magnetometer->getY() - (int16_t) getOptionValue(OPT_MAG_CAL_OFS);
			if (pump.doOffActions(30)) {
				Serial.print("Pump off, magfld low ");
				Serial.print(magfldLow);
				Serial.print(", high ");
				Serial.print(magfldHigh);
				Serial.print(", swing ");
				Serial.println(magfldHigh - magfldLow);

				magfldLow = magfldHigh = adjustedMagFld;
			}
			magfldLow = min (magfldLow, adjustedMagFld);
			magfldHigh = max (magfldHigh, adjustedMagFld);

			if (adjustedMagFld > getOptionValue(OPT_MAGFIELD_TRIP)) {
				uint16_t effectiveMagFld = adjustedMagFld - getOptionValue(OPT_MAGFIELD_TRIP);
				if (effectiveMagFld > MAX_FIELD_SWING) {
					Serial.println("Ignoring spurious mag fld reading");
				} else {
					Serial.print("mag sw, spd req: ");
					//say 0-240 -> 0-480 fair enough

					uint16_t spdReq = effectiveMagFld * getOptionValue(OPT_MAGFIELD_GAIN) / 10;
					pump.requestMinSpeed(spdReq);

					Serial.print("rq spd: ");
					Serial.print(spdReq);
					Serial.println();

					if (adjustedMagFld > getOptionValue(OPT_MAGFIELD_DANGER)) {
						Serial.print("Danger magfield! ");
						Serial.println(adjustedMagFld);
						dangerLevel = true;
					}
				}
			}
		}
	}
#endif
}
void checkPanic() {
	/*
	 * Panic control  -  check if the level has been danger for > 10s, if so switch the shower pump off
	 */
	timeSinceLastOk = ticks.getSecond() - timeLeveLastOk;

	//panic if time since last ok > OPT_MAXTIME_FULL_DRAIN
	timeToPanic = ((int) getOptionValue(OPT_MAXTIME_FULL_DRAIN)) - timeSinceLastOk;
	panicking = (timeToPanic <= 0);
	if (panicking) {
		Serial.print("Panicking! ");
		Serial.print((uint32_t) timeLeveLastOk);
		Serial.print(" lastOK, time ");
		Serial.print((uint32_t) ticks.getSecond());
		Serial.print("Max: ");
		Serial.print(((int) getOptionValue(OPT_MAXTIME_FULL_DRAIN)));
		Serial.println();
		digitalWrite(PIN_POWERSHOWER_PUMP_ENABLE, OC_OFF);
	}
}

void checkCalendar() {
	//Calendar things
	//daily calibration?
	DateTime now = rtc->now();
	int hour = now.hour();
	int minute = now.minute();
	int doW = now.dayOfWk();
#if USE_PRESSURE
	if (tMinLastCalibration > 60 * 24 && tMinLastActivity > 20) {
		if (hour > 3 && hour < 5) {
			oled->showCalibMsg();
			pressure.calibratePressureSensors();
			oled->showStatus();
		}
	}
#endif

#if ELSA_ALARM
	//Elsa alarm
	//we want the following to only work on weekdays, so DoW == 1-5 not 0 or 7. doW % 7 is 0 for Sunday and 6 for Saturday..

	if (hour == 14 && minute == 45 && doW != 0 && doW != 6 && !lights.getAlarm()) {
		lights.setAlarm(true);
	} else if (hour == 15 && minute == 30 && doW != 0 && doW != 6 && lights.getAlarm()) {
		lights.setAlarm(false);
	}
#endif
}

void detailedLogging() {
	bool broadcast = ticks.thisSecond % 5 == 0;

	LOG(" mg:");
	LOG(adjustedMagFld);
	LOG('/');
	LOG(getOptionValue(OPT_MAGFIELD_TRIP));

#if USE_PRESSURE
	LOG(" pD:");
	LOG(pressure.getMeanPressureDifferential());
#endif
	/*
	 LOG(" dT: ")
	 LOG(dhtGetTemp());

	 LOG(" wT:");
	 LOG(pressure.getDrainTemp());

	 LOG(" aT:");
	 LOG(pressure.getAirTemp());
	 */
	LOG(" Fl:");
	LOG(flowPulsesThisSecond);

	LOG(" Gp:");
	LOG(pump.leakyBucket);

	LOG(" Hu:");
	LOG(extractor.humidity);
	LOG('/');
	LOG(getOptionValue(OPT_HUMIDITY_THRESHOLD));
	/*
	 LOG(" Hu1:");
	 LOG(analogRead(PIN_ANA_HUMIDITY1));

	 LOG(" Hu2:");
	 LOG(analogRead(PIN_ANA_HUMIDITY2));

	 LOG(" aP:");
	 LOG(pressure.getAirPressure());

	 LOG(" wP:");
	 LOG(pressure.getDrainPressure());
	 */
	/* Display the results (magnetic vector values are in micro-Tesla (uT)) */
	/*
	 LOG(" X:");
	 LOG(magnetometer->getX());
	 LOG(" Y:");
	 LOG(magnetometer->getY());
	 LOG(" Z:");
	 LOG(magnetometer->getZ());
	 */
	LOG("$");
	Serial.println();
}

void handleEvents() {
	Message *msg = messageQueue.getNextMessage();
	if (msg) {
		switch (msg->event) {
		case Event::DHT_TIMEOUT_LOW:
			Serial.println("DHT TIMEOUT - is it even connected?");
//			showDhtState(); //			addSample(pDiff);
			if (dhtOk) {
				buttonLights.errorCode(ERROR_DHT_TIMEOUT_LOW);
				dhtOk = false;
			}
			break;

		case Event::DHT_TIMEOUT_HIGH:
			Serial.println("DHT TIMEOUT - is it even connected?");
//			showDhtState(); //			addSample(pDiff);
			if (dhtOk) {
				buttonLights.errorCode(ERROR_DHT_TIMEOUT_HIGH);
				dhtOk = false;
			}
			break;

		case Event::TIMER4:
			if (dhtOk) {
				buttonLights.errorCode(ERROR_TIMER4);
				dhtOk = false;
			}
			Serial.println("Timer4 TIMEOUT");
			break;

		case Event::DHT_CONVERSION:
//			Serial.println("DHT conversion");
			if (!dhtOk) {
				buttonLights.clearError();
				dhtOk = true;
			}
			extractor.recordHumidity();
			break;

		case Event::DHT_CHECKSUM:
			Serial.println("DHT Checksum error");
//			showDhtState();
			if (dhtOk) {
				buttonLights.errorCode(ERROR_DHT_CHECKSUM);
				dhtOk = false;
			}
			break;

		case Event::KEYPRESS_SHORT:
			Serial.println("Short keypress");
			oled->shortPress();
			break;

		default:
			Serial.print("Unknown msg:");
			Serial.println(msg->event);
			break;
		}
	}
}

void handleCommand() {
	if (Serial.available()) {
		int option;
		uint16_t value;
		uint16_t command = Serial.read();
		switch (command) {
		case '?':
			showCommands();
			break;

		case 'p':
			Serial.println(CMD_PUMP_KICK);
			Serial.print("kick sw");
			pump.requestMinSpeed(300);
			break;

		case 'l':
			Serial.println(CMD_LEAK_RATE);
			alterValue(OPT_PUMP_LEAKY_BUCKET_LEAK_RATE);
			break;

		case 'm':
			Serial.println(CMD_SET_MAG_TRIP);
			alterValue(OPT_MAGFIELD_TRIP);
			break;

		case 'o':
			Serial.println(CMD_UPDATE_UPTION);
			option = Serial.parseInt();
			alterValue(option);
			break;

		case 'x':
			Serial.println(CMD_EXTRACTOR_KICK);
			extractor.recalibrateToChangeState();
			break;

		case 'f':
			Serial.println(CMD_FAKE_PRESSURE);
#if PRESSURE_SENSOR
			pressure.setFakeDifferential(Serial.parseInt());
#endif
			break;

		case 'd':
			Serial.println("Diverter");
			diverter->cycle();
			break;

		case 's':
			Serial.println(CMD_RESYNC);
			oled->showCalibMsg();
#if PRESSURE_SENSOR
			pressure.calibratePressureSensors();
#endif
			oled->showStatus();
			break;

		case 'c':
			Serial.println(CMD_DIVERT);
			diverter->cycle();
			break;

		case 'r':
			Serial.println(CMD_RESET);
			interrupts.resetInterruptCounts();
			resetEepromToDefaults();
			break;

		case 'v':
			Serial.println(CMD_VIEW_SETTINGS);
			listCurrentValues();
			break;

		default:
			Serial.print("Unknown command ");
			Serial.println((char) command);
			showCommands();
			break;
		}
	}
}

void alterValue(int option) {
	uint16_t value = getOptionValue(option);
	Serial.print(getOptionName(option));
	Serial.print(": ");
	Serial.print(value);
	value = Serial.parseInt();
	setAndSaveOptionValue(option, value);
	Serial.print(" -> ");
	Serial.println(value);
}

void showCommands() {
	Serial.println("Commands:");
	Serial.println(CMD_PUMP_KICK);
	Serial.println(CMD_LEAK_RATE);
	Serial.println(CMD_EXTRACTOR_KICK);
	Serial.println(CMD_FAKE_PRESSURE);
	Serial.println(CMD_RESYNC);
}

