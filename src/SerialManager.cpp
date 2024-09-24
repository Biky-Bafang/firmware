#include "SerialManager.h"
#include "BLEManager.h"

// Constructor
SerialManager::SerialManager() : wifiStatus(0) {}

static const char *TAG = "SerialManager";
int packetDelayVar = 0;
TaskHandle_t SerialManager::taskHandle1 = NULL;
TaskHandle_t SerialManager::taskHandle2 = NULL;

// Initialize the SD card
void SerialManager::init(bool invert1, bool invert2, int packetDelay, LuaManager *luaManager, std::vector<flowData> *flowList)
{
	packetDelayVar = packetDelay;
	ESP_LOGI(TAG, "Initializing Serial");
	if (!invert1)
		Serial1.begin(1200, SERIAL_8N1, cRX1, cTX1);
	else
		Serial1.begin(1200, SERIAL_8N1, cTX1, cRX1);
	if (!invert2)
		Serial2.begin(1200, SERIAL_8N1, cRX2, cTX2);
	else
		Serial2.begin(1200, SERIAL_8N1, cTX2, cRX2);
	SerialParams *params = new SerialParams{luaManager, flowList, this};
	xTaskCreate(
		SerialManager::serial1Task, /* Task function. */
		"serial1Task",				/* String with name of task. */
		10000,						/* Stack size in bytes. */
		params,						/* Parameter passed as input of the task */
		2,							/* Priority of the task. */
		&taskHandle1);				/* Task handle. */
	xTaskCreate(
		SerialManager::serial2Task, /* Task function. */
		"serial2Task",				/* String with name of task. */
		10000,						/* Stack size in bytes. */
		params,						/* Parameter passed as input of the task */
		2,							/* Priority of the task. */
		&taskHandle2);				/* Task handle. */
}
void SerialManager::restart(bool invert1, bool invert2, int packetDelay)
{
	vTaskDelete(taskHandle1);
	vTaskDelete(taskHandle2);
	taskHandle1 = NULL;
	taskHandle2 = NULL;
	init(invert1, invert2, packetDelay, NULL, NULL);
}

void SerialManager::serial1Task(void *parameter)
{
	SerialParams *params = static_cast<SerialParams *>(parameter);
	LuaManager *luaManager = params->luaManager;
	std::vector<flowData> *flowList = params->flowList;
	SerialManager *instance = params->instance;

	uint8_t buffer[100];
	size_t bufferIndex = 0;
	unsigned long lastBuffer = millis();
	while (1)
	{
		if (Serial1.available())
		{
			int c = Serial1.read();
			if (c == -1)
				continue;
			buffer[strlen((char *)buffer)] = c;
			lastBuffer = millis();
			buffer[bufferIndex++] = (uint8_t)c;
			if (bufferIndex >= sizeof(buffer))
			{
				bufferIndex--;
			}
		}
		if (millis() - lastBuffer > packetDelayVar && bufferIndex > 0)
		{
			String bufferString;
			for (int i = 0; i < bufferIndex; i++)
			{
				String tempString;
				if (i == 0)
					tempString = "0x";
				else
					tempString = " 0x";
				if (buffer[i] < 0x10)
					tempString += "0";
				tempString += String(buffer[i], HEX);
				bufferString += tempString;
			}
			for (auto const &flow : *flowList)
			{
				if (flow.trigger_data == bufferString.c_str() && flow.trigger_device == "motor")
				{
					luaManager->run(flow);
					break;
				}
			}
			ESP_LOGI(TAG, "Serial1: %s", bufferString.c_str());
			bufferIndex = 0;
			memset(buffer, 0, sizeof(buffer));
		}
		vTaskDelay(3); // Yield to other tasks
	}
}
void SerialManager::serial2Task(void *parameter)
{
	SerialParams *params = static_cast<SerialParams *>(parameter);
	SerialManager *instance = params->instance;
	LuaManager *luaManager = params->luaManager;

	uint8_t buffer[100];
	size_t bufferIndex = 0;
	unsigned long lastBuffer = millis();

	while (1)
	{
		if (Serial2.available())
		{
			int c = Serial2.read();
			if (c == -1)
				continue;
			buffer[strlen((char *)buffer)] = c;
			lastBuffer = millis();
			buffer[bufferIndex++] = (uint8_t)c;
			if (bufferIndex >= sizeof(buffer))
			{
				bufferIndex--;
			}
		}
		if (millis() - lastBuffer > packetDelayVar && bufferIndex > 0)
		{
			JsonDocument tempDoc;
			String bufferString;
			for (int i = 0; i < bufferIndex; i++)
			{
				String tempString = " 0x";
				if (buffer[i] < 0x10)
					tempString += "0";
				tempString += String(buffer[i], HEX);
				bufferString += tempString;
			}
			for (auto const &flow : *params->flowList)
			{
				if (flow.trigger_data == bufferString.c_str() && flow.trigger_device == "display")
				{
					ESP_LOGI(TAG, "Found flow");
					luaManager->run(flow);
					break;
				}
			}
			tempDoc["type"] = "serial2";
			tempDoc["value"] = bufferString;
			String tempvalue;
			serializeJson(tempDoc, tempvalue);
			// TODO send to ble
			ESP_LOGI(TAG, "Serial2: %s", tempvalue.c_str());
			BLEManager &manager = BLEManager::getInstance();
			manager.sendData(tempDoc, false);

			bufferIndex = 0;
			memset(buffer, 0, sizeof(buffer));
		}
		vTaskDelay(5); // Yield to other tasks
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