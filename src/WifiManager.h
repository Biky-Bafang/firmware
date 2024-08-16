#ifndef WIFIMANAGER_H // Header guard start
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Arduino.h>
#include <RTOS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WebServer.h>

#define SSID "decop7"
#define PASSWORD "!10057704a"

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
	void init();

private:
	static void loop(void *parameter);
	void handleFileDownload(); // Function declaration for file download
	WebServer server{80};	   // HTTP server on port 80
};

#endif // Header guard end
