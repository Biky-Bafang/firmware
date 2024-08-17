#ifndef WIFIMANAGER_H // Header guard start
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Arduino.h>
#include <RTOS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Class definition

class WifiManager // Class declaration
{
public:
	WifiManager(); // Constructor declaration

	static WifiManager &getInstance()
	{
		static WifiManager instance;
		return instance;
	}

	WifiManager(WifiManager const &) = delete;
	void operator=(WifiManager const &) = delete;

	int wifiStatus;
	void init(JsonDocument *settings);
	void restart(JsonDocument *settings);

private:
	static void loop(void *parameter);
	void handleFileDownload(); // Function declaration for file download
	WebServer server{80};	   // HTTP server on port 80
	struct WifiParams
	{
		JsonDocument &settings;
		WifiManager *instance; // Add instance pointer for accessing non-static members
	};
	static TaskHandle_t taskHandle;
};

#endif // Header guard end
