#ifndef SETTINGSMANAGER_H // Header guard start
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <StreamUtils.h>

#define firmwareVersion "1.0.0"
#define hardwareVersion "1.0.0"

class SettingsManager // Class declaration
{
public:
	SettingsManager(); // Constructor declaration

	struct WifiParams
	{
		const char *ssid;
		const char *password;
	};

	static SettingsManager &getInstance()
	{
		static SettingsManager instance;
		return instance;
	}

	SettingsManager(SettingsManager const &) = delete;
	void operator=(SettingsManager const &) = delete;

	void init(JsonDocument *settings, JsonDocument *flows);
	JsonDocument settings;
	JsonDocument &getSettings() { return settings; }

private:
	static void loop(void *parameter);
};

#endif // Header guard end
