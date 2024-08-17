#include "WifiManager.h"
#include "SettingsManager.h"

static const char *TAG = "WifiManager";

// Constructor
WifiManager::WifiManager() : wifiStatus(0), server(80) {}

TaskHandle_t WifiManager::taskHandle = NULL;

// Initialize the WiFi and server
void WifiManager::init(JsonDocument *settings)
{

	ESP_LOGI(TAG, "Initializing WiFi");
	WiFi.mode(WIFI_STA);

	// Set up HTTP server routes
	server.on("/download", HTTP_GET, [this]()
			  { this->handleFileDownload(); });

	// Start the server
	server.begin();

	WifiParams *params = new WifiParams{*settings, this};
	// Start the WiFi manager task
	xTaskCreate(
		WifiManager::loop, /* Task function. */
		"WiFiManager",	   /* String with name of task. */
		10000,			   /* Stack size in bytes. */
		params,			   /* Parameter passed as input of the task */
		1,				   /* Priority of the task. */
		&taskHandle);	   /* Task handle. */
}

// make an restart function
void WifiManager::restart(JsonDocument *settings)
{
	vTaskDelete(taskHandle);
	taskHandle = NULL;
	(*settings)["wifiStatus"] = 0;
	WiFi.disconnect();
	init(settings);
}

void WifiManager::handleFileDownload()
{
	File file = LittleFS.open("/variables.json", "r");
	if (!file)
	{
		server.send(500, "text/plain", "Failed to open file for reading");
		return;
	}

	server.streamFile(file, "application/octet-stream");
	file.close();
}

void WifiManager::loop(void *parameter)
{
	WifiParams *params = static_cast<WifiParams *>(parameter);
	JsonDocument *settings = &params->settings;
	int time = 0;
	while (true)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			(*settings)["wifiStatus"] = 1;

			if (millis() - time > 8000)
			{
				time = millis();
				ESP_LOGI(TAG, "Signal strength: %d dBm", WiFi.RSSI());
			}
			delay(1000);
			continue;
		}
		if ((*settings)["wifiStatus"] != 0)
		{
			ESP_LOGI(TAG, "Retrying in 5 seconds");
			(*settings)["wifiStatus"] = 0;
			delay(5000);
		}
		ESP_LOGI(TAG, "Connecting to WiFi");
		(*settings)["wifiStatus"] = 0;
		// use ssid and password from init
		const char *ssid = (*settings)["ssid"];
		const char *password = (*settings)["password"];
		WiFi.begin(ssid, password);
		int i = millis();
		while (WiFi.status() != WL_CONNECTED && millis() - i < 10000 && WiFi.status() != WL_CONNECT_FAILED)
		{
			delay(500);
		}
		if (WiFi.status() != WL_CONNECTED)
		{
			(*settings)["wifiStatus"] = 2;
			WiFi.disconnect();
			ESP_LOGE(TAG, "WiFi connection failed");
		}
		else
		{
			(*settings)["wifiStatus"] = 1;
			ESP_LOGI(TAG, "WiFi connected");
			ESP_LOGI(TAG, "Connected to %s", ssid);
			ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
			(*settings)["ip"] = WiFi.localIP();
			ESP_LOGI(TAG, "Signal strength: %d dBm", WiFi.RSSI());
		}
	}
}
