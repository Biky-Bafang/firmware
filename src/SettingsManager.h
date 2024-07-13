#ifndef SETTINGSMANAGER_H // Header guard start
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <StreamUtils.h>

class SettingsManager // Class declaration
{
public:
	SettingsManager(); // Constructor declaration
	void init();
	JsonDocument settings;

private:
	static void loop(void *parameter);
};

#endif // Header guard end
