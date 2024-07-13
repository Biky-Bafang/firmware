#ifndef SERIALMANAGER_H // Header guard start
#define SERIALMANAGER_H

#include <Arduino.h>
#include <RTOS.h>

#define cRX1 9
#define cTX1 10
#define cRX2 12
#define cTX2 11

// Class definition

class SerialManager // Class declaration
{
public:
	SerialManager(); // Constructor declaration
	void init();
	int wifiStatus; // 0: Disconnected, 1: Connected, 2: Connection failed

private:
	static void serial1ToSerial2(void *parameter);
	static void serial2ToSerial1(void *parameter);
};

#endif // Header guard end
