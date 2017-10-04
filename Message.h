#ifndef Message_h
#define Message_h

#include <Arduino.h>

#define TRUE ((boolean) true)
#define FALSE ((boolean) false)


/* Namespace seems to be a fair compromise since Enum doesn't behave consistently over the 2 compilers */
namespace Event {
enum Event {
	TICK,
	MEASURE_PRESSURE,
	PANIC,
	DHT_TIMEOUT_HIGH,
	DHT_TIMEOUT_LOW,
	DHT_CHECKSUM,
	DHT_CONVERSION,
	TIMER4,
	KEYPRESS_SHORT,
//	KEYPRESS_LONG,
};
}

class Message {
public:
	void *data;
	Event::Event event;
};

class MessageQueue {
	static const int MESSAGE_QUEUE_SIZE = 32;
	Message messages[MESSAGE_QUEUE_SIZE];
	int wPtr, rPtr;

	inline boolean empty() {
		return (wPtr == rPtr);
	}

	/*
	 * full if rPtr is 1 more than wPtr
	 */
	inline boolean full() {
		return (rPtr - wPtr + MESSAGE_QUEUE_SIZE) % MESSAGE_QUEUE_SIZE == 1;
	}

public:
	inline MessageQueue() {
		wPtr = 0;
		rPtr = 0;
	}

	boolean sendMessage(Event::Event event, void *data) {
		if (full()) {
			Serial.println("MESSAGE QUEUE FULL!");
			return false;
		}
		messages[wPtr].event = event;
		messages[wPtr].data = data;
		wPtr = (wPtr + 1) % MESSAGE_QUEUE_SIZE;
		return TRUE;
	}
	boolean sendMessage(Event::Event event) {
		return sendMessage(event, NULL);
	}

	Message *getNextMessage() {
		if (empty()) {
			return NULL;
		}
		Message *result = &(messages[rPtr]);
		rPtr = (rPtr + 1) % MESSAGE_QUEUE_SIZE;
		return result;
	}
};

extern MessageQueue messageQueue;

#endif

