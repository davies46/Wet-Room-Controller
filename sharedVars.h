/*
 * sharedVars.h
 *
 *  Created on: 31 Aug 2014
 *      Author: pdavies
 */

#ifndef SHAREDVARS_H_
#define SHAREDVARS_H_

#include <Arduino.h>
#include "Extractor.h"
#include "Pressure.h"
#include "RTClib.h"
#include "Ticks.h"
#include "Pump.h"
#include "Lights.h"
#include "Diverter.h"
#include "Message.h"
#include "UI.h"
#include "FlowMeter.h"
#include "Constants.h"
#include "ButtonLights.h"
#include "TX433.h"

#define UINT8_T 1
#define UINT16_T 2
#define FLT_T 3
#define UINT32_T 4
#define BOOL_T 5

extern MessageQueue messageQueue;
extern UI *oled;
extern Diverter *diverter;
extern ButtonLights buttonLights;

extern RTC_DS1307 *rtc;
#if PRESSURE_SENSOR
extern Pressure pressure;
#endif
extern long int adjustedMagFld;
extern Extractor extractor;
extern Ticks ticks;
extern FlowMeter *flowMeter;
extern TX433 *tx433;

extern float meanPressureDifferential;
extern uint32_t spuriousPressureDifferentialCount;
extern uint32_t maxAbsSpuriousPressureDif;

extern bool panicking, floatA, floatB;

extern Pump pump;
extern Lights lights;

extern uint64_t tMinLastCalibration;
extern uint64_t tMinLastActivity;
extern int16_t maxDeltaFlowPulses;

extern uint16_t numDhtConversions;

extern int timeToPanic;
extern int timeSinceLastOk;

extern int16_t dialCnt;

//extern uint8_t nextOutputMode[];

extern uint16_t dhtNumErrors;
extern uint8_t dhtLastError;

#endif /* SHAREDVARS_H_ */
