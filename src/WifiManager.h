#ifndef WIFIMANAGER_H // Header guard start
#define WIFIMANAGER_H

#include <WiFi.h>
#include <Arduino.h>
#include <RTOS.h>

#define SSID "decop7"
#define PASSWORD "!10057704a"

// Class definition

class WifiManager // Class declaration
{
public:
	WifiManager(); // Constructor declaration
	void init();
	int wifiStatus; // 0: Disconnected, 1: Connected, 2: Connection failed

private:
	static void loop(void *parameter);
};

#endif // Header guard end
