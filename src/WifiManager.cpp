#include "WifiManager.h"

// Constructor
WifiManager::WifiManager() : wifiStatus(0) {}

static const char *TAG = "WifiManager";

// Initialize the SD card
void WifiManager::init()
{
	ESP_LOGI(TAG, "Initializing WiFi");
	WiFi.mode(WIFI_STA);
	xTaskCreate(
		WifiManager::loop, /* Task function. */
		"WiFiManager",	   /* String with name of task. */
		10000,			   /* Stack size in bytes. */
		this,			   /* Parameter passed as input of the task */
		1,				   /* Priority of the task. */
		NULL);			   /* Task handle. */
}

void WifiManager::loop(void *parameter)
{
	WifiManager *instance = static_cast<WifiManager *>(parameter);
	while (true)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			instance->wifiStatus = 1;
			delay(1000);
			continue;
		}
		if (instance->wifiStatus != 0)
		{
			ESP_LOGI(TAG, "Retrying in 5 seconds");
			instance->wifiStatus = 0;
			delay(5000);
		}
		ESP_LOGI(TAG, "Connecting to WiFi");
		instance->wifiStatus = 0;
		WiFi.begin(SSID, PASSWORD);
		int i = millis();
		while (WiFi.status() != WL_CONNECTED && millis() - i < 10000 && WiFi.status() != WL_CONNECT_FAILED)
		{
			delay(500);
		}
		if (WiFi.status() != WL_CONNECTED)
		{
			instance->wifiStatus = 2;
			WiFi.disconnect();
			ESP_LOGE(TAG, "WiFi connection failed");
		}
		else
		{
			instance->wifiStatus = 1;
			ESP_LOGI(TAG, "WiFi connected");
		}
	}
}
