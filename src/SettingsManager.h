#ifndef SETTINGSMANAGER_H // Header guard start
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <StreamUtils.h>
#include <vector>
#include <LittleFS.h>

#define flowVersion 1
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
	std::string name;				 // Name of the flow
	std::string id;					 // ID
	std::string trigger_type;		 // Type of trigger (e.g., "CODE", "TIME", "SENSOR")
	std::string trigger_data;		 // Trigger data (e.g., "0x11, 0x52", "500", "SENSOR_ID")
	std::string trigger_device;		 // Device that triggers the flow (e.g., "DISPLAY", "MOTOR") - Only if trigger_type is "CODE"
	std::string lua_code;			 // Lua code as a string
	std::vector<Variable> variables; // Variables needed for the Lua code, including value and persistence
	bool coreFlow;					 // Whether the flow is a core flow (i.e., a built-in flow that cannot be removed)
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
	std::string readString(fs::File &file);
	void writeString(fs::File &file, const std::string &str);
	bool dumpFlowDataToBinary(fs::File &file, const flowData &flow);
	bool readFlowDataFromBinary(fs::File &file, flowData &flow);

	SettingsManager(SettingsManager const &) = delete;
	void operator=(SettingsManager const &) = delete;

	void init(JsonDocument *settings, std::vector<flowData> *flowList);
	JsonDocument settings;
	JsonDocument &getSettings() { return settings; }

private:
	static void loop(void *parameter);
};

#endif // Header guard end
