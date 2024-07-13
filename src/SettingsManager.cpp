#include "SettingsManager.h"

// Constructor
SettingsManager::SettingsManager() : settings() {}

static const char *TAG = "SettingsManager";

// Initialize the SD card
void SettingsManager::init()
{
	ESP_LOGI(TAG, "Initializing Settings");
	EEPROM.begin(512);
	EepromStream eepromStream(0, 512);
	DeserializationError error = deserializeJson(settings, eepromStream);
	if (error || settings.size() <= 5)
	{
		settings["name"] = "Biky 12f";
		settings["password"] = "12345678";
		settings["packetDelay"] = 10;
		settings["txPower"] = 9;
		settings["invertSerial1"] = false;
		settings["invertSerial2"] = false;
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
	SettingsManager *instance = static_cast<SettingsManager *>(parameter);
	while (true)
	{
		vTaskDelay(5000);
	}
}
