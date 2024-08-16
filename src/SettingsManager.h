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

	static SettingsManager &getInstance()
	{
		static SettingsManager instance;
		return instance;
	}

	SettingsManager(SettingsManager const &) = delete;
	void operator=(SettingsManager const &) = delete;

	void init();
	JsonDocument settings;
	JsonDocument &getSettings() { return settings; }

private:
	static void loop(void *parameter);
};

#endif // Header guard end
