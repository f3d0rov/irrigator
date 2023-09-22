
#pragma once

#include <util/delay.h>

// From DS1302 datasheet; 5V voltage assumed

// RST to CLK setup
#define DS1302_T_CC_us 4.0
// DATA to CLK setup
#define DS1302_T_DC_ns 50.0
// CLK to DATA hold
#define DS1302_T_CDH_ns 70.0
// CLK to DATA delay
#define DS1302_T_CDD_ns 100.0
// CLK HIGH time
#define DS1302_T_CH_us 1.0
// CLK LOW time
#define DS1302_T_CL_us 1.0
// CLK to RST hold
#define DS1302_T_CCH_ns 60.0
// RST inactive time
#define DS1302_T_CWH_us 1.0
// CLK to Data max delay
#define DS1302_T_CDD_ns 200.0
// CLK to I/O HIGH Z max
#define DS1302_T_CCZ_ns 70.0


// Calendar/clock registers
#define DS1302_REG_SECONDS 	0b10000000
#define DS1302_REG_MINUTES 	0b10000010
#define DS1302_REG_HOURS	0b10000100
#define DS1302_REG_DATE		0b10000110
#define DS1302_REG_MONTH	0b10001000
#define DS1302_REG_DAY		0b10001010
#define DS1302_REG_YEAR		0b10001100


struct Datetime {
	int year = 0, month = 0, date = 0;
	int hours = 0, minutes = 0, seconds = 0;
	int day = 0;

	Datetime();
	Datetime (int year, int month, int date, int hs, int min, int sec);
	Datetime (long unix);

	String dateString();
	String timeString();
	String toString();
	
	static bool isLeapYear (int y);
	long unix();

	bool operator< (Datetime &right);
	Datetime operator+ (long seconds);
	Datetime operator- (long seconds);
};

// DS1302 Serial clock module communication class.
class DS1302 {
		int _clk, _dat, _rst;
		bool _running;

		void writeRawByte (byte b); // Write a byte to a pin. RST pin assumed HIGH
		void writeByte (byte reg, byte data); // Write a byte to a register
		byte readByte (byte reg); // Read a byte from a register

		// Get command byte for a RAM register
		static byte RAM_reg (int addr);

		// Get command byte for a clock/calendar data register.
		static byte CK_reg (int addr);

		// Converters for DS1302 binary coded decimal format

		static byte getBits (byte from, int first, int last); // Get bits in positions [first, last] (including both ends)

		static int convert60Base (byte from);
		static int convertHours (byte from);
		static int convertDate (byte from);
		static int convertMonth (byte from);
		static int convertDay (byte from);
		static int convertYear (byte from);

		byte convertToSeconds (int from);
		static byte convertToMinutes (int from);
		static byte convertToHours (int from);
		static byte convertToDate (int from);
		static byte convertToMonth (int from);
		static byte convertToDay (int from);
		static byte convertToYear (int from);
	public:
		DS1302 (int clk, int dat, int rst);

		Datetime getDatetime ();
		void setupDatetime (Datetime dt);

		void setRamByte (int addr, byte val);
		byte getRamByte (int addr);

		bool getClockHaltBit ();
		void setClockHaltBit (bool state);
		bool isRunning();
};

