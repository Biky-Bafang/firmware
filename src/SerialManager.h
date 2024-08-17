#ifndef SERIALMANAGER_H // Header guard start
#define SERIALMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>

#define cRX1 9
#define cTX1 10
#define cRX2 4
#define cTX2 5

// Class definition

class SerialManager // Class declaration
{
public:
	SerialManager(); // Constructor declaration
	void init(bool invert1, bool invert2, int packetDelay);
	void restart(bool invert1, bool invert2, int packetDelay);
	int wifiStatus; // 0: Disconnected, 1: Connected, 2: Connection failed

private:
	static void serial1ToSerial2(void *parameter);
	static void serial2ToSerial1(void *parameter);
	static TaskHandle_t taskHandle1;
	static TaskHandle_t taskHandle2;
};

#endif // Header guard end
