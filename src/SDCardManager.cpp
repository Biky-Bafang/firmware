// #include "SDCardManager.h"

// // Constructor
// SDCardManager::SDCardManager() {}

// // Initialize the SD card
// void SDCardManager::init()
// {
// 	if (!SD_MMC.begin())
// 	{
// 		Serial.println("Card Mount Failed");
// 		return;
// 	}
// 	uint8_t cardType = SD_MMC.cardType();
// 	if (cardType == CARD_NONE)
// 	{
// 		Serial.println("No SD card attached");
// 		return;
// 	}
// 	Serial.println("SD card initialized");
// 	// create rtos task
// 	xTaskCreate(
// 		SDCardManager::loop, /* Task function. */
// 		"SDCardManager",	 /* name of task. */
// 		10000,				 /* Stack size of task */
// 		NULL,				 /* parameter of the task */
// 		1,					 /* priority of the task */
// 		NULL);				 /* Task handle to keep track of created task */
// }

// SDCardManager::loop(void *parameter)
// {
// 	while (true)
// 	{
// 		delay(500);
// 	}
// }