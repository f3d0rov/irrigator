
#pragma once

class Relay {
		int _pin;
	public:
		Relay (int signalPin);

		void enable();
		void disable();
		void set (bool enable);
};
