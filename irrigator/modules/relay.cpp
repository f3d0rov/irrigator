
#include "relay.hpp"

Relay::Relay (int signalPin) {
	this->_pin = signalPin;
	pinMode(this->_pin, OUTPUT);
}

void Relay::enable() {
	digitalWrite(this->_pin, HIGH);
}

void Relay::disable() {
	digitalWrite(this->_pin, LOW);
}

void Relay::set (bool enable) {
	if (enable) {
		this->enable();
	} else {
		this->disable();
	}
}

