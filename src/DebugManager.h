
#ifndef DEBUG_MANAGER_H
#define DEBUG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// make an debug manager class
class DebugManager
{
public:
	DebugManager();
	String getColor(String color);
	void print(String type, String infoMessage, String color);

private:
};

#endif
