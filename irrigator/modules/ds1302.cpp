
#include "ds1302.hpp"

Datetime::Datetime () {

}

Datetime::Datetime (int year, int month, int date, int hs, int min, int sec):
year (year), month (month), date (date), hours (hs), minutes (min), seconds (sec) {

}

bool Datetime::operator< (Datetime &right) {
	return this->year < right.year | this->month < right.month | this->day < right.day | this->hours < right.hours | this->minutes < right.minutes | this->seconds < right.seconds;
}

void DS1302::writeRawByte (byte b) {
	for (int i = 0; i < 8; i++) {
		bool bit = (b >> i) & 1; // Get bit to write (stating with bit 0)
		digitalWrite (this->_dat, bit ? HIGH : LOW); // Write bit to DAT
		_delay_us (DS1302_T_CL_us); // CLK LOW delay
		
		// _delay_us (DS1302_T_DC_ns / 1000); // 50ns or higher is required by DS1302 datasheet
		digitalWrite (this->_clk, HIGH); // Rising edge on CLK
		_delay_us (DS1302_T_CH_us); // CLK HIGH delay
		digitalWrite (this->_clk, LOW); // Falling edge on CLK
	}
}

void DS1302::writeByte (byte reg, byte data) { 
	digitalWrite (this->_clk, LOW); // CLK must be LOW when RST becomes HIGH
	digitalWrite (this->_rst, HIGH); // RST is high during entirety of transmission

	_delay_us (DS1302_T_CC_us); // Timings provided by DS1302 datasheet
	byte writeReg = (reg | (1 << 7)) & ~((byte) 1); // Bit 7 is set to enable writes to the DS1302; bit 0 is reset to specify a write operation
	writeRawByte (writeReg); // Write command byte
	writeRawByte (data);	// Write data

	_delay_us (DS1302_T_CCH_ns / 1000); // CLK to RST hold delay
	digitalWrite (this->_rst, LOW); // RST to LOW
	_delay_us (DS1302_T_CWH_us); // wait min RST inactive time
}

byte DS1302::readByte (byte reg) {
	digitalWrite (this->_clk, LOW);
	digitalWrite (this->_rst, HIGH); // RST is high to initiate data transfer

	_delay_us (DS1302_T_CC_us);
	byte readReg = (reg | (1 << 7)) | 1; // `The MSB (Bit 7) must be a logic 1.` - DS1302 datasheet. Bit 0 is set to specify a read operation
	writeRawByte (readReg);
	pinMode (this->_dat, INPUT); // Get ready to read data


	_delay_us(DS1302_T_CDD_ns / 2000); // Wait for DS1302 to transmit data	
	byte result = 0;
	for (int i = 0; i < 8; i++) {
		bool bit = digitalRead(this->_dat); // Read bit i
		if (bit) result |= 1 << i; // Set result bit if 1

		digitalWrite (this->_clk, HIGH); // Rising edge on CLK
		_delay_us (DS1302_T_CCZ_ns / 2000); // Wait
		digitalWrite (this->_clk, LOW); // Falling edge on CLK
		_delay_us (DS1302_T_CDD_ns / 2000); // Wait for next bit
	}

	digitalWrite (this->_rst, LOW);
	pinMode (this->_dat, OUTPUT); // Default mode for data pin is OUTPUT
	_delay_us (DS1302_T_CWH_us); // Wait min RST inactive time

	return result;
}

byte DS1302::RAM_reg (int addr) {
	return (addr << 1) | (1 << 6); // lshift addr 1 bit, set bit 6
}

byte DS1302::CK_reg (int addr) {
	return (addr << 1) & ~(1 << 6); // lshift addr 1 bit, ensure bit 6 is reset
}

byte DS1302::getBits (byte from, int first, int last) {
	// Assume a byte like b0iii0000; i marks the bits that interest us
	// In this case `from` is the byte, `first` is 4 and `last` is 6
	// (from >> first) shifts the interesting bits to be the first bits in the byte
	// ~((byte) 0) << (last - first + 1) is a mask (b11111000) that has 0 for each of the interesting bits, 1 for rest
	// We invert it to b00000111 (1 for interesting bits) and apply to the shifted byte, leaving us with the result of b00000iii. 
	return (from >> first) & ~(~((byte) 0) << (last - first + 1));
}

int DS1302::convert60Base (byte from) {
	return 10 * DS1302::getBits(from, 4, 6) + DS1302::getBits(from, 0, 3);
}

int DS1302::convertHours (byte from) {
	return 10 * DS1302::getBits(from, 4, 5) + DS1302::getBits(from, 0, 3);
}

int DS1302::convertDate (byte from) {
	return 10 * DS1302::getBits(from, 4, 5) + DS1302::getBits(from, 0, 3);
}

int DS1302::convertMonth (byte from) {
	return 10 * DS1302::getBits(from, 4, 4) + DS1302::getBits(from, 0, 3);
}

int DS1302::convertDay (byte from) {
	return DS1302::getBits (from, 0, 2) - 1;
}

int DS1302::convertYear (byte from) {
	return 2000 + 10 * DS1302::getBits (from, 4, 7) + DS1302::getBits (from, 0, 3);
}

byte DS1302::convertToSeconds(int from) {
	int lt10 = from % 10;
	return (this->_running ? 0 : 1 << 7) + lt10 + ((from / 10) << 4);
}

byte DS1302::convertToMinutes(int from) {
	int lt10 = from % 10;
	return lt10 + ((from / 10) << 4);
}

byte DS1302::convertToHours (int from) {
	int lt10 = from % 10;
	return ((from / 10) << 4) + lt10;
}

byte DS1302::convertToDate (int from) {
	int lt10 = from % 10;
	return ((from / 10) << 4) + lt10;
}

byte DS1302::convertToMonth (int from) {
	int lt10 = from % 10;
	return ((from / 10) << 4) + lt10;
}

byte DS1302::convertToDay (int from) {
	return from + 1;
}

byte DS1302::convertToYear (int from) {
	from -= 2000;
	int lt10 = from % 10;
	return ((from / 10) << 4) + lt10;
}


DS1302::DS1302 (int clk, int dat, int rst)
: _clk (clk), _dat (dat), _rst (rst) {
	pinMode (_clk, OUTPUT);
	pinMode (_dat, OUTPUT); // DAT pin is used as input when reading. The default state for it is OUTPUT and it is switched to INPUT on reads.
	pinMode (_rst, OUTPUT);

	digitalWrite (_clk, LOW);
	digitalWrite (_dat, LOW);
	digitalWrite (_rst, LOW);

	this->_running = !this->getClockHaltBit(); // Check if the clock is running on startup - writes to seconds register depend on it
}

Datetime DS1302::getDatetime () {
	Datetime result;
	result.seconds = DS1302::convert60Base (this->readByte (DS1302_REG_SECONDS));
	result.minutes = DS1302::convert60Base (this->readByte (DS1302_REG_MINUTES));
	result.hours = DS1302::convertHours (this->readByte (DS1302_REG_HOURS));
	result.date = DS1302::convertDate (this->readByte (DS1302_REG_DATE));
	result.month = DS1302::convertMonth (this->readByte (DS1302_REG_MONTH));
	result.day = DS1302::convertDay (this->readByte (DS1302_REG_DAY));
	result.year = DS1302::convertYear (this->readByte (DS1302_REG_YEAR));
	return result;
}

void DS1302::setupDatetime (Datetime dt) {
	this->writeByte (DS1302_REG_SECONDS, this->convertToSeconds (dt.seconds));
	this->writeByte (DS1302_REG_MINUTES, DS1302::convertToMinutes (dt.minutes));
	this->writeByte (DS1302_REG_HOURS, DS1302::convertToHours (dt.hours)); 
	this->writeByte (DS1302_REG_DATE, DS1302::convertToDate (dt.date));
	Serial.print("0b");
	Serial.println(DS1302::convertToDate (dt.date), BIN);
	this->writeByte (DS1302_REG_MONTH, DS1302::convertToMonth (dt.month));
	this->writeByte (DS1302_REG_YEAR, DS1302::convertToYear (dt.year));
	// DS1302 should figure day of the week on its own
}

byte DS1302::getRamByte (int addr) {
	return this->readByte (DS1302::RAM_reg(addr));
}

void DS1302::setRamByte (int addr, byte val) {
	this->writeByte (DS1302::RAM_reg(addr), val);
}

bool DS1302::getClockHaltBit () {
	byte seconds = this->readByte (DS1302::CK_reg(DS1302_REG_SECONDS)); // Get current seconds byte
	return (seconds >> 7) & 1;
}

void DS1302::setClockHaltBit (bool state) {
	byte seconds = this->readByte (DS1302::CK_reg(DS1302_REG_SECONDS)); // Get current seconds byte
	if (state) seconds |= 1 << 7; // Set/reset clock halt bit
	else seconds &= ~(1 << 7);
	this->writeByte (DS1302_REG_SECONDS, seconds); // Write seconds byte with clock halt bit adjusted
	this->_running = ! state;
}

bool DS1302::isRunning () {
	return this->_running;
}
