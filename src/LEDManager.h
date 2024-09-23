#ifndef LEDMANAGER_H // Header guard start
#define LEDMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <RTOS.h>

#define NEOPIXEL_PIN 48
#define FADE_TRANSITION_TIME 250 // Time in milliseconds for fading transitions

// Class definition
class LEDManager
{
public:
	LEDManager(); // Constructor declaration
	void init();
	void setLED(int r, int g, int b, int brightness, int mode);
	void setLED(int r, int g, int b, int brightness);
	void setBrightness(int brightness);

private:
	static void loop(void *parameter);
	struct LEDParams
	{
		LEDManager *instance; // Add instance pointer for accessing non-static members
	};
	static TaskHandle_t taskHandle;
};

#endif // Header guard end
