#include <Arduino.h>
#include "BLEManager.h"
#include "SDCardManager.h"
#include "WifiManager.h"
#include "SettingsManager.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include "esp_log.h"

BLEManager bleManager;
SDCardManager sdCardManager;
WifiManager wifiManager;
SettingsManager settingsManager;

static const char *TAG = "Main";

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic) override
	{
		std::string value = pCharacteristic->getValue();
		if (value[0] == 0xf1)
		{
			value.erase(0, 1);
			if (value[0] == 0x01)
			{
				bleManager.sendData(settingsManager.settings);
				return;
			}
			if (value[0] == 0x02)
			{
				ESP.restart();
				return;
			}
			if (value[0] == 0x03)
			{
				settingsManager.settings.clear();
				EepromStream eepromStream(0, 512);
				serializeJson(settingsManager.settings, eepromStream);
				EEPROM.commit();
				return;
			}
			if (value[0] == 0x04)
			{
				JsonDocument responseDoc;
				deserializeJson(responseDoc, "{\"sd\":[],\"core\":[{\"id\":\"Core-0\",\"icon\":\"faBluetoothB\",\"label\":\"Start\",\"nodes\":5,\"triggers\":1}]}");
				bleManager.sendData(responseDoc);
				return;
			}
		}
		else if (value[0] == 0xf2)
		{
			value.erase(0, 1);
			if (value[0] == 0x01)
			{
				settingsManager.settings["name"] = value.substr(1);
			}
			if (value[0] == 0x02)
			{
				settingsManager.settings["password"] = value.substr(1);
			}
			if (value[0] == 0x03)
			{
				settingsManager.settings["packetDelay"] = int(value[1]);
			}
			if (value[0] == 0x04)
			{
				settingsManager.settings["txPower"] = int(value[1]);
			}
			if (value[0] == 0x05)
			{
				// check if invertSerial1 is 1 or 0
				settingsManager.settings["invertSerial1"] = value[1] == 0x01;
			}
			if (value[0] == 0x06)
			{
				// check if invertSerial2 is 1 or 0
				settingsManager.settings["invertSerial2"] = value[1] == 0x01;
			}
			// save tghe settingsManager.settings to EEPROM
			EepromStream eepromStream(0, 512);
			serializeJson(settingsManager.settings, eepromStream);
			EEPROM.commit();
		}
		else if (value[0] == 0xf3)
		{
			value.erase(0, 1);
			JsonDocument responseDoc;
			responseDoc["type"] = "response";
			if (value[0] == 0x01) // Read General
			{
				responseDoc["response"] = 0x52;
				uint8_t buffer[2] = {0x11, 0x52};
				Serial2.write(buffer, 2);
			}
			if (value[0] == 0x02) // Read Pedal
			{
				responseDoc["response"] = 0x53;
				uint8_t buffer[2] = {0x11, 0x53};
				Serial2.write(buffer, 2);
			}
			if (value[0] == 0x03) // Read Throttle
			{
				responseDoc["response"] = 0x54;
				uint8_t buffer[2] = {0x11, 0x54};
				Serial2.write(buffer, 2);
			}
			// current time
			unsigned long startTime = millis();
			while (1)
			{
				if (bleManager.dataCache.size() > 0 && bleManager.dataCache[0].containsKey("type"))
				{
					JsonDocument tempDoc;
					deserializeJson(tempDoc, bleManager.dataCache[0]);
					responseDoc["value"] = tempDoc["value"];
					bleManager.sendData(responseDoc);
					// if startTime is greater than 1000ms break
					if (millis() - startTime > 1000)
						break;
					break;
				}
				vTaskDelay(5);
			}
			return;
		}
		else if (value[0] == 0xf4)
		{
			value.erase(0, 1);
			JsonDocument tempDoc;
			if (value[0] == 0x01)
			{
				value.erase(0, 1);
				tempDoc["type"] = "serial1 (S)";
				Serial1.write(value.c_str(), value.length());
			}
			else if (value[0] == 0x02)
			{
				value.erase(0, 1);
				tempDoc["type"] = "serial2 (S)";
				Serial2.write(value.c_str(), value.length());
			}
			// send to ble the feedback
			String bufferString;
			for (int i = 0; i < value.length(); i++)
			{
				String tempString = " 0x";
				if (value[i] < 0x10)
					tempString += "0";
				tempString += String(value[i], HEX);
				bufferString += tempString;
			}
			tempDoc["value"] = bufferString;
			bleManager.sendData(tempDoc);
		}
	}

	void onRead(BLECharacteristic *pCharacteristic) override
	{
		// Handle read request
	}
};

// create an task to send the data to ble

// create an task so when it recieves on Serial it will send it to Serial1

void otaTask(void *parameter)
{
	while (1)
	{
		ArduinoOTA.handle();
		vTaskDelay(1); // Yield to other tasks
	}
}

void setup()
{
	Serial.begin(115200);
	esp_log_level_set("*", ESP_LOG_VERBOSE);
	delay(1000);
	ESP_LOGI(TAG, "Starting...");

	settingsManager.init();
	sdCardManager.init();
	bleManager.init(settingsManager.settings["name"], new MyCharacteristicCallbacks(), bleManager.powerLevel(settingsManager.settings["txPower"]));
	wifiManager.init();

	// xTaskCreate(otaTask, "OTA", 8000, NULL, 1, NULL);
}

void loop()
{
	delay(500);
	settingsManager.settings["sdCard"] = sdCardManager.sdCardStatus;
}
