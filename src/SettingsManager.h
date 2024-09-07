#ifndef SETTINGSMANAGER_H // Header guard start
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <StreamUtils.h>

#define firmwareVersion "1.0.0"
#define hardwareVersion "1.0.0"
struct Variable
{
	std::string name;
	std::string type;  // Example: "integer", "string", "boolean"
	std::string value; // Current value of the variable (stored as a string for simplicity)
	bool persist;	   // Whether the variable should be persisted across reboots
};

struct flowData
{
	std::string id;						   // ID
	std::vector<uint8_t> hex_trigger_code; // Hex trigger code (e.g., 0x11, 0x52)
	std::string lua_code;				   // Lua code as a string
	std::vector<Variable> variables;	   // Variables needed for the Lua code, including value and persistence
};

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

	void init(JsonDocument *settings);
	JsonDocument settings;
	JsonDocument &getSettings() { return settings; }

private:
	static void loop(void *parameter);
};

#endif // Header guard end
