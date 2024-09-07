#include "SDCardManager.h"
static const char *TAG = "SDCardManager";

// Constructor
SDCardManager::SDCardManager() {}
TaskHandle_t SDCardManager::taskHandle = NULL;

// Initialize the SD card
void SDCardManager::init(JsonDocument *settings)
{
	ESP_LOGI(TAG, "Initializing SD card");
	pinMode(CD, INPUT_PULLDOWN);
	SD_MMC.setPins(CLK, CMD, DATA0);

	sdCardParams *params = new sdCardParams{*settings, this};
	// create rtos task
	xTaskCreate(
		SDCardManager::loop, /* Task function. */
		"SDCardManager",	 /* String with name of task. */
		10000,				 /* Stack size in bytes. */
		params,				 /* Parameter passed as input of the task */
		1,					 /* Priority of the task. */
		&taskHandle);		 /* Task handle. */
}

void SDCardManager::restart(JsonDocument *settings)
{
	vTaskDelete(taskHandle);
	taskHandle = NULL;
	init(settings);
}

void SDCardManager::loop(void *parameter)
{
	sdCardParams *params = static_cast<sdCardParams *>(parameter);
	JsonDocument *settings = &params->settings;

	while (true)
	{
		// Now you can access instance variables using `instance->`
		if (digitalRead(CD) == HIGH && ((*settings)["sdCardStatus"] == 0 || (*settings)["sdCardStatus"] == 2))
		{
			(*settings)["sdCardStatus"] = 1;
			ESP_LOGI(TAG, "SD card inserted");
			// initialize SD card
			if (!SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 4))
			{
				(*settings)["sdCardStatus"] = 2;
				ESP_LOGE(TAG, "Card Mount Failed");
				delay(1000);
				continue;
			}
			ESP_LOGI(TAG, "SD Card Type: %s", SD_MMC.cardType() == CARD_NONE ? "No SD card attached" : SD_MMC.cardType() == CARD_MMC ? "MMC"
																								   : SD_MMC.cardType() == CARD_SD	 ? "SDSC"
																								   : SD_MMC.cardType() == CARD_SDHC	 ? "SDHC"
																																	 : "UNKNOWN");
		}
		else if (digitalRead(CD) == LOW && (*settings)["sdCardStatus"] != 0)
		{
			(*settings)["sdCardStatus"] = 0;
			ESP_LOGI(TAG, "SD card removed");

			SD_MMC.end();
		}
		delay(500);
	}
}