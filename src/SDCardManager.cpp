#include "SDCardManager.h"
static const char *TAG = "SDCardManager";

// Constructor
SDCardManager::SDCardManager() : sdCardStatus(0) {}

// Initialize the SD card
void SDCardManager::init()
{
	ESP_LOGI(TAG, "Initializing SD card");

	pinMode(CD, INPUT_PULLDOWN);
	SD_MMC.setPins(CLK, CMD, DATA0);

	// create rtos task
	xTaskCreate(
		SDCardManager::loop, /* Task function. */
		"SDCardManager",	 /* String with name of task. */
		10000,				 /* Stack size in bytes. */
		this,				 /* Parameter passed as input of the task */
		1,					 /* Priority of the task. */
		NULL);				 /* Task handle. */
}

void SDCardManager::loop(void *parameter)
{
	SDCardManager *instance = static_cast<SDCardManager *>(parameter);
	while (true)
	{
		// Now you can access instance variables using `instance->`

		if (digitalRead(CD) == HIGH && (instance->sdCardStatus == 0 || instance->sdCardStatus == 2))
		{
			instance->sdCardStatus = 2;
			ESP_LOGI(TAG, "SD card inserted");
			// initialize SD card
			if (!SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 4))
			{
				ESP_LOGE(TAG, "Card Mount Failed");
				delay(1000);
				continue;
			}
			instance->sdCardStatus = 1;
		}
		else if (digitalRead(CD) == LOW && instance->sdCardStatus != 0)
		{
			instance->sdCardStatus = 0;
			ESP_LOGI(TAG, "SD card removed");

			SD_MMC.end();
		}
		delay(500);
	}
}
