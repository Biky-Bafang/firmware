#include "SerialManager.h"

// Constructor
SerialManager::SerialManager() : wifiStatus(0) {}

static const char *TAG = "SerialManager";

// Initialize the SD card
void SerialManager::init()
{
	ESP_LOGI(TAG, "Initializing Serial");
	Serial1.begin(1200, SERIAL_8N1, cRX1, cTX1);
	Serial2.begin(1200, SERIAL_8N1, cRX2, cTX2);
	xTaskCreate(
		SerialManager::serial1ToSerial2, /* Task function. */
		"serial1ToSerial2",				 /* String with name of task. */
		10000,							 /* Stack size in bytes. */
		this,							 /* Parameter passed as input of the task */
		1,								 /* Priority of the task. */
		NULL);							 /* Task handle. */
	xTaskCreate(
		SerialManager::serial2ToSerial1, /* Task function. */
		"serial2ToSerial1",				 /* String with name of task. */
		10000,							 /* Stack size in bytes. */
		this,							 /* Parameter passed as input of the task */
		1,								 /* Priority of the task. */
		NULL);							 /* Task handle. */
}

void SerialManager::serial1ToSerial2(void *parameter)
{
	SerialManager *instance = static_cast<SerialManager *>(parameter);
	while (true)
	{
		if (Serial1.available())
		{
			Serial2.write(Serial1.read());
		}
		vTaskDelay(1);
	}
}
void SerialManager::serial2ToSerial1(void *parameter)
{
	SerialManager *instance = static_cast<SerialManager *>(parameter);
	while (true)
	{
		if (Serial2.available())
		{
			Serial1.write(Serial2.read());
		}
		vTaskDelay(1);
	}
}

// void serial2ToSerial1(void *parameter)
// {
// 	uint8_t buffer[100];
// 	size_t bufferIndex = 0;
// 	bool usedPattern = false;
// 	unsigned long lastBuffer = millis();

// 	while (1)
// 	{
// 		if (Serial2.available())
// 		{
// 			int c = Serial2.read();
// 			if (c == -1)
// 				continue;
// 			Serial1.write(c);

// 			buffer[strlen((char *)buffer)] = c;
// 			lastBuffer = millis();
// 			buffer[bufferIndex++] = (uint8_t)c;
// 			if (bufferIndex >= sizeof(buffer))
// 			{
// 				bufferIndex--;
// 			}
// 		}
// 		if (millis() - lastBuffer > bufferTimeDelay && bufferIndex > 0)
// 		{
// 			JsonDocument tempDoc;
// 			String bufferString;
// 			for (int i = 0; i < bufferIndex; i++)
// 			{
// 				String tempString = " 0x";
// 				if (buffer[i] < 0x10)
// 					tempString += "0";
// 				tempString += String(buffer[i], HEX);
// 				bufferString += tempString;
// 			}
// 			tempDoc["type"] = "serial2";
// 			tempDoc["value"] = bufferString;
// 			String tempvalue;
// 			serializeJson(tempDoc, tempvalue);
// 			dataCache.add(tempvalue);
// 			bufferIndex = 0;
// 			memset(buffer, 0, sizeof(buffer));
// 		}
// 		vTaskDelay(1); // Yield to other tasks
// 	}
// }

// void serial1ToSerial2(void *parameter)
// {
// 	uint8_t buffer[100];
// 	size_t bufferIndex = 0;
// 	bool usedPattern = false;
// 	unsigned long lastBuffer = millis();

// 	while (1)
// 	{
// 		if (Serial1.available())
// 		{
// 			int c = Serial1.read();
// 			if (c == -1)
// 				continue;

// 			// if buffer is "0x16 0x1a" if it is send 0xf1 otherwise send c
// 			if (buffer[0] == 0x16 && buffer[1] == 0x1a)
// 			{
// 				Serial2.write(0xf1);
// 			}
// 			else
// 			{
// 				Serial2.write(c);
// 			}
// 			buffer[strlen((char *)buffer)] = c;
// 			lastBuffer = millis();
// 			buffer[bufferIndex++] = (uint8_t)c;
// 			if (bufferIndex >= sizeof(buffer))
// 			{
// 				bufferIndex--;
// 			}
// 		}
// 		if (millis() - lastBuffer > bufferTimeDelay && bufferIndex > 0)
// 		{
// 			JsonDocument tempDoc;
// 			String bufferString;
// 			for (int i = 0; i < bufferIndex; i++)
// 			{
// 				String tempString = " 0x";
// 				if (buffer[i] < 0x10)
// 					tempString += "0";
// 				tempString += String(buffer[i], HEX);
// 				bufferString += tempString;
// 			}
// 			tempDoc["type"] = "serial1";
// 			tempDoc["value"] = bufferString;
// 			String tempvalue;
// 			serializeJson(tempDoc, tempvalue);
// 			dataCache.add(tempvalue);
// 			bufferIndex = 0;
// 			memset(buffer, 0, sizeof(buffer));
// 		}
// 		vTaskDelay(1); // Yield to other tasks
// 	}
// }