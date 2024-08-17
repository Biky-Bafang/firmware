#include <Arduino.h>
#include "BLEManager.h"
#include "SDCardManager.h"
#include "WifiManager.h"
#include <LittleFS.h>
#include "SettingsManager.h"
#include "SerialManager.h"
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
SerialManager serialManager;
JsonDocument settings;

static const char *TAG = "Main";

JsonDocument getVariables()
{
	JsonDocument doc;

	File file = LittleFS.open("/variables.bin", "r");
	if (!file)
	{
		ESP_LOGE(TAG, "Failed to open file for reading");
		return doc;
	}

	ESP_LOGI(TAG, "File Content:");
	// the file looks like: variable=value\n so split it by = and put it in the doc. so {variable: value} and each \n is a new variable and it has
	while (file.available())
	{
		String variables = file.readString();
		deserializeJson(doc, variables);
	}
	return doc;
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
	void onWrite(BLECharacteristic *pCharacteristic) override
	{
		std::string value = pCharacteristic->getValue();
		// print the value to console with hex
		String result;
		for (int i = 0; i < value.length(); i++)
		{
			// if value[0] == 0xf2 and i is more than 1 then add it as ascii instead of hex
			if (value[0] == 0xf2 && i > 1)
			{
				if (i == 2)
					result += " ";
				result += value[i];
				continue;
			}
			String tempString = " 0x";
			if (value[i] < 0x10)
				tempString += "0";
			tempString += String(value[i], HEX);
			result += tempString;
		}
		ESP_LOGI(TAG, "Received: %s", result.c_str());
		if (value[0] == 0xf1)
		{
			value.erase(0, 1);
			if (value[0] == 0x01)
			{
				bleManager.sendData(&settings);
				return;
			}
			if (value[0] == 0x02)
			{
				ESP.restart();
				return;
			}
			if (value[0] == 0x03)
			{
				settings.clear();
				EepromStream eepromStream(0, 512);
				serializeJson(settings, eepromStream);
				EEPROM.commit();
				return;
			}
			if (value[0] == 0x04)
			{
				JsonDocument responseDoc;
				deserializeJson(responseDoc, "{\"sd\":[]}");
				JsonDocument tempDoc;
				deserializeJson(tempDoc, "{\"id\":\"Core-0\",\"icon\":\"faBluetoothB\",\"label\":\"Start\",\"nodes\":5,\"triggers\":1}");
				responseDoc["core"].add(tempDoc);
				responseDoc["variables"] = getVariables();
				serializeJsonPretty(responseDoc, Serial);
				bleManager.sendData(responseDoc);
				return;
			}
		}
		else if (value[0] == 0xf2)
		{
			value.erase(0, 1);
			if (value[0] == 0x00)
			{
				settings["name"] = value.substr(1);
			}
			if (value[0] == 0x01)
			{
				settings["ssid"] = value.substr(1);
				wifiManager.restart(&settings);
			}
			if (value[0] == 0x02)
			{
				settings["password"] = value.substr(1);
				wifiManager.restart(&settings);
			}
			if (value[0] == 0x03)
			{
				settings["packetDelay"] = int(value[1]);
				serialManager.restart(settings["invertSerial1"], settings["invertSerial2"], settings["packetDelay"]);
			}
			if (value[0] == 0x04)
			{
				settings["txPower"] = int(value[1]);
				bleManager.setPowerLevel(bleManager.powerLevel(settings["txPower"]));
			}
			if (value[0] == 0x05)
			{
				settings["invertSerial1"] = value[1] == 0x01;
				serialManager.restart(settings["invertSerial1"], settings["invertSerial2"], settings["packetDelay"]);
			}
			if (value[0] == 0x06)
			{
				// check if invertSerial2 is 1 or 0
				settings["invertSerial2"] = value[1] == 0x01;
				serialManager.restart(settings["invertSerial1"], settings["invertSerial2"], settings["packetDelay"]);
			}
			// save the settings to EEPROM
			EepromStream eepromStream(0, 512);
			serializeJson(settings, eepromStream);
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

void setup()
{
	Serial.begin(115200);
	delay(500);
	ESP_LOGI(TAG, "Starting...");
	if (!LittleFS.begin())
	{
		ESP_LOGE(TAG, "Failed to mount file system");
		return;
	}

	settingsManager.init(&settings);
	serializeJsonPretty(settings, Serial);
	sdCardManager.init(&settings);
	serialManager.init(settings["invertSerial1"], settings["invertSerial2"], settings["packetDelay"]);
	bleManager.init(settings["name"], new MyCharacteristicCallbacks(), bleManager.powerLevel(settings["txPower"]));
	wifiManager.init(&settings);
}
void loop()
{
	// ArduinoOTA.handle();
	vTaskDelete(NULL);
}
