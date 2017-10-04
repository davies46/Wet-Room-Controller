#include "DHT22.h"

#define DHT_DEVICE 22

const uint32_t DHT_IDLE = 0;
const uint32_t DHT_INIT = 1;
const uint32_t DHT_START_SIGNAL = 2;
const uint32_t DHT_START_SIGNAL_END = 4;
const uint32_t DHT_START_RESPONSE = 8;
const uint32_t DHT_WAIT_HIGH = 16;
const uint32_t DHT_WAIT_LOW = 32;
const uint32_t DHT_COOLDOWN = 64;

void dhtChangeIsr();
void setDhtState(byte state);
void timer3delayUs(uint16_t us);
void timer3delayMs(uint16_t ms);

uint32_t dhtState = DHT_IDLE;
uint32_t statesEntered;
bool dhtPresent = true;
uint16_t timer3Start;
int timer3count;

int dhtWord;
int dhtBit;
byte dhtCurrent;
byte dhtData[6];
byte dhtChecksum;
int lowBitCnt;
int highBitCnt;
bool gotStartBit;
uint16_t numDhtConversions = 0;
uint16_t dhtNumErrors = 0;
uint8_t dhtLastError = 0;

uint16_t humidityLow = 0xFFFF;
uint16_t tempLow = 0xFFFF;

void dhtError(Event::Event error) {
	messageQueue.sendMessage(error, dhtData);
	dhtLastError = error;
	dhtNumErrors++;
	setDhtState(DHT_IDLE);
	dhtPresent = false;
	pinMode(PIN_DHT_IO, OUTPUT);
	digitalWrite(PIN_DHT_IO, LOW);
}

void clearDhtData() {
	for (int i = 0; i < 5; i++) {
		dhtData[i] = 0;
	}
}

void handleDhtEvent() {
	switch (dhtState) {
	case DHT_IDLE:
		break;

	case DHT_INIT:
		//do stuff
		pinMode(PIN_DHT_IO, OUTPUT);
		digitalWrite(PIN_DHT_IO, HIGH);
		setDhtState(DHT_START_SIGNAL);
		clearDhtData();
		timer3delayUs(250);
		break;

	case DHT_START_SIGNAL:
		digitalWrite(PIN_DHT_IO, LOW);
		setDhtState(DHT_START_SIGNAL_END);
		timer3delayMs(20);
		break;

	case DHT_START_SIGNAL_END:
		digitalWrite(PIN_DHT_IO, HIGH);
		setDhtState(DHT_START_RESPONSE);
		pinMode(PIN_DHT_IO, INPUT);
		timer3delayUs(50);
		break;

	case DHT_START_RESPONSE:
		setDhtState(DHT_WAIT_HIGH);
		gotStartBit = 0;
		dhtWord = dhtCurrent = dhtChecksum = 0;
		lowBitCnt = highBitCnt = 0;
		dhtBit = 7;
		attachInterrupt(INTERRUPT_LINE_DHT, dhtChangeIsr, RISING);
		timer3delayMs(110);
		break;

	case DHT_WAIT_HIGH:
		dhtError(Event::DHT_TIMEOUT_HIGH);
		break;

	case DHT_WAIT_LOW:
		dhtError(Event::DHT_TIMEOUT_LOW);
		break;

	case DHT_COOLDOWN:
		pinMode(PIN_DHT_IO, OUTPUT);
		digitalWrite(PIN_DHT_IO, HIGH);
		setDhtState(DHT_IDLE);
		break;

	default:
		break;
	}
}

void showDhtState() {
	Serial.print(" bit:");
	Serial.print(dhtBit);
	Serial.print(" word:");
	Serial.print(dhtWord);
	Serial.print(" statesEntered:");
	Serial.print(statesEntered, HEX);
	Serial.print(" bitsLH:");
	Serial.print(lowBitCnt);
	Serial.print(",");
	Serial.print(highBitCnt);
	Serial.println();

	for (int b = 0; b < 5; b++) {
		Serial.println(dhtData[b], HEX);
	}
	Serial.print("CS ");
	Serial.println(dhtData[5], HEX);
	Serial.print("State ");
	Serial.println(dhtState);
}

char getDhtStateSymbol() {
	switch (dhtState) {
	case DHT_IDLE:
		return 'i';
	case DHT_INIT:
		return 'n';
	case DHT_START_SIGNAL:
		return 's';
	case DHT_START_SIGNAL_END:
		return 'S';
	case DHT_START_RESPONSE:
		return 'r';
	case DHT_WAIT_HIGH:
		return 'w';
	case DHT_WAIT_LOW:
		return 'W';
	case DHT_COOLDOWN:
		return 'C';
	default:
		return 'X';
	}
}

void setDhtState(byte state) {
	dhtState = state;
	statesEntered |= state;
}

void startDHT() {
	TCCR3B = 0x00; //Disable Timer
	dhtPresent = true;
	statesEntered = 0;
	setDhtState(DHT_INIT);
	handleDhtEvent();
}

uint16_t dhtGetHumidity() {
	if (!dhtPresent) {
		//it may come back or get plugged in or something, who knows?
		dhtPresent = true;
		clearDhtData();
		return 0;
	}
	uint16_t humidity;
#if DHT_DEVICE == 11
	humidity = dhtData[0];
#else
	humidity = dhtData[0] << 8 | dhtData[1];
#endif

	humidityLow = min(humidity, humidityLow);
	return humidity;
}

float dhtGetTemp() {
	if (!dhtPresent) {
		//it may come back or get plugged in or something, who knows?
		dhtPresent = true;
		clearDhtData();
		return 0;
	}
	float temp;
#if DHT_DEVICE == 11
	temp = dhtData[1];
#else
	temp = (dhtData[2] & 0x7F) << 8 | dhtData[3];
#endif

	tempLow = min(temp, tempLow);
	return temp / 10.0;
}

uint16_t dhtGetTemp10(){
	if (!dhtPresent) {
		//it may come back or get plugged in or something, who knows?
		dhtPresent = true;
		clearDhtData();
		return 0;
	}

	uint16_t temp = (dhtData[2] & 0x7F) << 8 | dhtData[3];
	return temp;
}

//Timer3 Overflow Interrupt Vector, called with varying times
ISR(TIMER3_OVF_vect) {
	timer3count++;
	TCNT3 = timer3Start; //Preset Timer Count to 130 out of 255
	if (dhtState != DHT_IDLE) {
		TCCR3B = 0x00; //Disable Timer
		handleDhtEvent();
	}
}

void timer3delayMs(uint16_t ms) {
	TCCR3B = 0x00; //Disable Timer3 while we set it up
	TCNT3 = timer3Start = 65536l - 16l * ms; //Preset Timer Count
	TIFR3 = 0x00; //Timer3 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK3 = 0x01; //Timer3 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR3A = 0x00; //Timer3 Control Reg A: Wave Gen Mode normal
	TCCR3B = 0x05; //16kHz (64us per tick)
}

void timer3delayUs(uint16_t us) {
	TCCR3B = 0x00; //Disable Timer3 while we set it up
	TCNT3 = timer3Start = 65536l - us * 2; //Preset Timer Count
	TIFR3 = 0x00; //Timer3 INT Flag Reg: Clear Timer Overflow Flag
	TIMSK3 = 0x01; //Timer3 INT Reg: Timer2 Overflow Interrupt Enable
	TCCR3A = 0x00; //Timer3 Control Reg A: Wave Gen Mode normal
	TCCR3B = 0x02; //2MHz (0.5us per tick)
}

void dhtChangeIsr() {
	switch (dhtState) {
	case DHT_WAIT_HIGH: // the response should have been low for ~50us, and now has gone high indicating the start of a data bit
		// set the next state, which will be waiting for the end of the data bit
		setDhtState(DHT_WAIT_LOW);
		// set a timeout for 10ms, but we want the timer to count in usec, so use usec timer
		timer3delayUs(11000);
		//come back to this isr when line goes low again
		attachInterrupt(INTERRUPT_LINE_DHT, dhtChangeIsr, FALLING);
		break;

	case DHT_WAIT_LOW: //whole bit came in
		setDhtState(DHT_WAIT_HIGH);
		int timerCnt = TCNT3 - timer3Start;
		timer3delayUs(10000); //this timeout is for the 50us gap
		attachInterrupt(INTERRUPT_LINE_DHT, dhtChangeIsr, RISING);
		if (!gotStartBit) {
			gotStartBit = 1;
			break;
		}
		//decide if high/low and handle
		byte b = (timerCnt > 80) ? 1 : 0;
		if (b) {
			highBitCnt++;
		} else {
			lowBitCnt++;
		}
		dhtCurrent |= b << dhtBit;
		if (dhtBit-- == 0) {
			dhtBit = 7;

			dhtData[dhtWord++] = dhtCurrent;
			if (dhtWord == 5) {
				//finished
				if (dhtChecksum != dhtCurrent) {
					dhtData[5] = dhtChecksum;
					dhtError(Event::DHT_CHECKSUM);
				} else {
					messageQueue.sendMessage(Event::DHT_CONVERSION, dhtData);
					numDhtConversions++;
				}
				setDhtState(DHT_COOLDOWN);
				timer3delayMs(200);
				detachInterrupt(3);
			} else {
				dhtChecksum += dhtCurrent;
			}
			dhtCurrent = 0;
		}
		break;
	}
}

