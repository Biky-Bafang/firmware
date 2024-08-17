#include "SettingsManager.h"

// Constructor
SettingsManager::SettingsManager() {}

static const char *TAG = "SettingsManager";

void SettingsManager::init(JsonDocument *settings)
{
	ESP_LOGI(TAG, "Initializing Settings");
	EEPROM.begin(512);
	EepromStream eepromStream(0, 512);
	DeserializationError error = deserializeJson(*settings, eepromStream);
	if (error || (*settings).size() < 7)
	{
		// check if (*settings)["name"] exists and is not empty
		(*settings)["name"] = (*settings)["name"] | "Biky";
		(*settings)["ssid"] = (*settings)["ssid"] | "changeme";
		(*settings)["password"] = (*settings)["password"] | "changeme";
		(*settings)["packetDelay"] = (*settings)["packetDelay"] | 10;
		(*settings)["txPower"] = (*settings)["txPower"] | 9;
		(*settings)["invertSerial1"] = (*settings)["invertSerial1"] | false;
		(*settings)["invertSerial2"] = (*settings)["invertSerial2"] | false;
	}
	xTaskCreate(
		SettingsManager::loop, /* Task function. */
		"SettingsManager",	   /* String with name of task. */
		10000,				   /* Stack size in bytes. */
		this,				   /* Parameter passed as input of the task */
		1,					   /* Priority of the task. */
		NULL);				   /* Task handle. */
}

void SettingsManager::loop(void *parameter)
{
	while (true)
	{
		vTaskDelay(5000);
	}
}