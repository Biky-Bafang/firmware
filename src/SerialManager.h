#ifndef SERIALMANAGER_H // Header guard start
#define SERIALMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>
#include "LuaManager.h"

#define cRX1 11
#define cTX1 12
#define cRX2 10
#define cTX2 9

// Class definition

class SerialManager // Class declaration
{
public:
	SerialManager(); // Constructor declaration
	void init(bool invert1, bool invert2, int packetDelay, LuaManager *luaManager);
	void restart(bool invert1, bool invert2, int packetDelay);
	int wifiStatus; // 0: Disconnected, 1: Connected, 2: Connection failed

private:
	static void serial1ToSerial2(void *parameter);
	static void serial2ToSerial1(void *parameter);
	static TaskHandle_t taskHandle1;
	static TaskHandle_t taskHandle2;
	struct SerialParams
	{
		LuaManager *luaManager;
		SerialManager *instance; // Add instance pointer for accessing non-static members
	};
};

#endif // Header guard end
