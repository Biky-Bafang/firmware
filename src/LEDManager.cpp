#include "LEDManager.h"
static const char *TAG = "LEDManager";

Adafruit_NeoPixel neopixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Constructor
LEDManager::LEDManager() {}
TaskHandle_t LEDManager::taskHandle = NULL;
int mode = 0;
uint32_t color = 0;

// Initialize the SD card
void LEDManager::init()
{
	LEDParams *params = new LEDParams{this};
	neopixel.begin();
	// create rtos task
	xTaskCreate(
		LEDManager::loop, /* Task function. */
		"LEDManager",	  /* String with name of task. */
		10000,			  /* Stack size in bytes. */
		params,			  /* Parameter passed as input of the task */
		1,				  /* Priority of the task. */
		&taskHandle);	  /* Task handle. */
}
void LEDManager::setLED(int r, int g, int b, int brightness, int setMode)
{
	mode = setMode;
	color = neopixel.Color(r, g, b);
	neopixel.setBrightness(brightness);
}
void LEDManager::setLED(int r, int g, int b, int brightness)
{
	LEDManager::setLED(r, g, b, brightness, 0);
}

void LEDManager::setBrightness(int brightness)
{
	neopixel.setBrightness(brightness);
}

void LEDManager::loop(void *parameter)
{
	// LEDParams *params = static_cast<LEDParams *>(parameter);
	// LEDManager *instance = params->instance;

	// Track the previous values we want to compare against
	int previousBrightness = 0;
	uint32_t previousColor = 0;

	while (true)
	{
		// Check if the color or brightness has changed
		if (color != previousColor || neopixel.getBrightness() != previousBrightness)
		{
			ESP_LOGI(TAG, "Changing color and brightness");

			// Extract RGB components of current and target color
			uint8_t prevRed = (previousColor >> 16) & 0xFF;
			uint8_t prevGreen = (previousColor >> 8) & 0xFF;
			uint8_t prevBlue = previousColor & 0xFF;

			uint8_t newRed = (color >> 16) & 0xFF;
			uint8_t newGreen = (color >> 8) & 0xFF;
			uint8_t newBlue = color & 0xFF;

			// Calculate the difference between the current and target color
			int redDiff = newRed - prevRed;
			int greenDiff = newGreen - prevGreen;
			int blueDiff = newBlue - prevBlue;

			// Get the current and target brightness
			int newBrightness = neopixel.getBrightness();
			int brightnessDiff = newBrightness - previousBrightness;

			// Gradually transition to the new color and brightness over 100 steps
			for (int i = 0; i <= 100; i++)
			{
				// Apply exponential easing to make the transition smoother
				float factor = powf((float)i / 100, 2); // Exponential curve, adjust exponent as needed

				// Interpolate color exponentially
				uint8_t fadeRed = prevRed + (redDiff * factor);
				uint8_t fadeGreen = prevGreen + (greenDiff * factor);
				uint8_t fadeBlue = prevBlue + (blueDiff * factor);

				// Interpolate brightness exponentially
				int fadeBrightness = previousBrightness + (brightnessDiff * factor);

				// Set the faded color and brightness
				neopixel.setPixelColor(0, neopixel.Color(fadeRed, fadeGreen, fadeBlue));
				neopixel.setBrightness(fadeBrightness);
				neopixel.show();
				delay(FADE_TRANSITION_TIME / 100); // Adjust this value to control the fading speed
			}

			// Update the tracked previousColor and previousBrightness with the new intended values
			previousColor = color;				// This should be the intended color, not fetched from neopixel
			previousBrightness = newBrightness; // Update with the actual brightness set
		}

		// Wait for color or brightness change
		while (color == previousColor && neopixel.getBrightness() == previousBrightness)
		{
			delay(60);
		}
	}
}
int waitWithoutChange(int ms, int lastMode)
{
	int waitedTime = 0;
	while (ms > 0 && waitedTime < ms)
	{
		if (lastMode != mode)
		{
			ESP_LOGI(TAG, "Mode changed, exiting");
			return 0;
		}
		delay(10);
		waitedTime += 10;
	}
	return 1;
}
