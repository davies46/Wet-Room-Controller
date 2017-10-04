/*
 * FlowMeter.cpp
 *
 *  Created on: 3 Sep 2014
 *      Author: pdavies
 */
#include <Arduino.h>
#include "pins.h"
#include "timerUtil.h"
#include "sharedVars.h"
#include "FlowMeter.h"

FlowMeter::FlowMeter() {
	initFlowMeter();
}

FlowMeter::~FlowMeter() {
}

volatile uint64_t flowTicks;
uint64_t secondsWhenStopped;
uint64_t t;

void flowEvent() {
	secondsWhenStopped = ticks.thisSecond;
	flowTicks++;
}

uint64_t FlowMeter::timeStoppedInSeconds() {
	return ticks.thisSecond - secondsWhenStopped;
}

void FlowMeter::initFlowMeter() {
	flowTicks = 0;
	t = 0;
	cycle = 0;
	secondsWhenStopped = ticks.thisSecond;
	attachInterrupt(INTERRUPT_LINE_FLOWMETER, flowEvent, CHANGE);
}

uint64_t FlowMeter::deltaTicks() {
	if (cycle++ % 4 == 0) {
		t = flowTicks;
		flowTicks = 0;
	}
	return t;
}
