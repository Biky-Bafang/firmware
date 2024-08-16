#include "WifiManager.h"
#include "SettingsManager.h"

static const char *TAG = "WifiManager";

// Constructor
WifiManager::WifiManager() : wifiStatus(0), server(80) {}

// Initialize the WiFi and server
void WifiManager::init()
{
	ESP_LOGI(TAG, "Initializing LittleFS");
	if (!LittleFS.begin())
	{
		ESP_LOGE(TAG, "Failed to mount LittleFS");
		return;
	}

	ESP_LOGI(TAG, "Initializing WiFi");
	WiFi.mode(WIFI_STA);

	// Set up HTTP server routes
	server.on("/download", HTTP_GET, [this]()
			  { this->handleFileDownload(); });

	// Start the server
	server.begin();

	// Start the WiFi manager task
	xTaskCreate(
		WifiManager::loop, /* Task function. */
		"WiFiManager",	   /* String with name of task. */
		10000,			   /* Stack size in bytes. */
		this,			   /* Parameter passed as input of the task */
		1,				   /* Priority of the task. */
		NULL);			   /* Task handle. */
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
	WifiManager *instance = static_cast<WifiManager *>(parameter);
	int time = 0;
	while (true)
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			instance->wifiStatus = 1;

			if (millis() - time > 8000)
			{
				time = millis();
				ESP_LOGI(TAG, "Signal strength: %d dBm", WiFi.RSSI());
			}
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
			ESP_LOGI(TAG, "Connected to %s", SSID);
			ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
			ESP_LOGI(TAG, "Signal strength: %d dBm", WiFi.RSSI());
			// get public IP
			HTTPClient http;
			http.begin("http://api.ipify.org");
			int httpCode = http.GET();
			if (httpCode > 0)
			{
				String payload = http.getString();
				ESP_LOGI(TAG, "Public IP address: %s", payload.c_str());
			}
			http.end();
		}
	}
}
