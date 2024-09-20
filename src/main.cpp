#include <Arduino.h>
#include "BLEManager.h"
#include "SDCardManager.h"
#include "WifiManager.h"
#include <LittleFS.h>
#include "SettingsManager.h"
#include "SerialManager.h"
#include "LuaManager.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <SD_MMC.h>
#include "esp_log.h"
#include <unordered_map>
#include <Adafruit_NeoPixel.h>

#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include "lua/rtoslib.h"
#include "lua/esplib.h"

extern "C"
{
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

BLEManager bleManager;
SDCardManager sdCardManager;
WifiManager wifiManager;
SettingsManager settingsManager;
SerialManager serialManager;
LuaManager luaManager;
#define NEOPIXEL_PIN 48

JsonDocument settings;
std::vector<flowData> flowList;

static const char *TAG = "Main";

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
			if ((value[0] == 0xf2) && i > 1)
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
				// put the variables to the responseDoc like [{"id": "value"}] without all the labels and such just that json
				// for (auto const &[key, val] : variables)
				// {
				// 	JsonDocument tempDoc;
				// 	tempDoc["id"] = key;
				// 	tempDoc["value"] = val;
				// 	responseDoc["variables"].add(tempDoc);
				// }
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
		// flow control
		else if (value[0] == 0xf5)
		{
			value.erase(0, 1);
			JsonDocument tempDoc;
			if (value[0] == 0x01) // Create an variable
			{
				value.erase(0, 1);
				if (value.empty())
					return;
				// variables[value] = 0;
				// save the variables to littlefs
				File file = LittleFS.open("/variables.bin", "w");
				if (!file)
				{
					ESP_LOGE(TAG, "Failed to open file for writing");
					return;
				}
				// for (const auto &[key, val] : variables)
				// {
				// 	uint16_t keyLength = key.length();
				// 	file.write((uint8_t *)&keyLength, sizeof(keyLength)); // Write length of the key
				// 	file.write((uint8_t *)key.c_str(), keyLength);		  // Write key data
				// 	file.write((uint8_t *)&val, sizeof(val));			  // Write value
				// 	file.write(0x00);									  // Null byte as delimiter
				// }
				file.close();
			}

			if (value[0] == 0x02) // Remove an variable
			{
				value.erase(0, 1);
				// variables.erase(value);
				// save the variables to littlefs
				File file = LittleFS.open("/variables.bin", "w");
				if (!file)
				{
					ESP_LOGE(TAG, "Failed to open file for writing");
					return;
				}
				// for (auto const &[key, val] : variables)
				// {
				// 	file.write((uint8_t *)key.c_str(), key.length());
				// 	file.write((uint8_t *)&val, sizeof(val) + 1);
				// }
				file.close();
			}

			if (value[0] == 0x03) // Edit an variable
			{
				value.erase(0, 1);
			}

			if (value[0] == 0x11) // Create a flow
			{
				value.erase(0, 1);
			}

			if (value[0] == 0x12) // Remove a flow
			{
				value.erase(0, 1);
			}

			if (value[0] == 0x13) // Edit a flow
			{
				value.erase(0, 1);
			}
		}
	}

	void onRead(BLECharacteristic *pCharacteristic) override
	{
		// Handle read request
	}
};
int maxHeapSize = 0;
void setup()
{
	// set neopixel to green at 128 brightness
	Adafruit_NeoPixel neopixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
	neopixel.begin();
	neopixel.setPixelColor(0, neopixel.Color(0, 128, 128));
	neopixel.setBrightness(128);
	neopixel.show();
	maxHeapSize = ESP.getFreeHeap();
	Serial.begin(115200);
	delay(1000);
	ESP_LOGI(TAG, "Starting...");
	if (!LittleFS.begin())
	{
		ESP_LOGE(TAG, "Failed to mount file system");
		return;
	}

	settingsManager.init(&settings, &flowList);
	serializeJsonPretty(settings, Serial);
	// sdCardManager.init(&settings); SD card not working on rev 0.2 :(
	serialManager.init(settings["invertSerial1"], settings["invertSerial2"], settings["packetDelay"], &luaManager);
	bleManager.init(settings["name"], new MyCharacteristicCallbacks(), bleManager.powerLevel(settings["txPower"]));
	wifiManager.init(&settings);
	luaManager.init(&flowList);

	flowData flow;
	flow.name = "Core-0";
	flow.id = "0";
	flow.trigger_type = "CODE";
	flow.trigger_data = "0x11, 0x52";
	flow.trigger_device = "MOTOR";
	flow.lua_code = "esp.logi('LuaExample','Sending 0x52, 0x01 for ok!')\nserial.write('motor', string.char(0x52, 0X01))\n";
	Variable var;
	var.name = "value";
	var.type = "string";
	var.value = "0x52, 0x01";
	var.persist = false;
	flow.variables.push_back(var);
	File file2 = LittleFS.open("/flows/0.bin", "r");

	// Open file for reading
	file2 = LittleFS.open("/flows/0.bin", "r");
	if (!file2)
	{
		ESP_LOGE(TAG, "Failed to open file for reading");
		return;
	}

	// Read flowData from file
	flowData tempFlow;
	if (!settingsManager.readFlowDataFromBinary(file2, tempFlow))
	{
		ESP_LOGE(TAG, "Failed to read flow data from binary");
		file2.close(); // Ensure the file is closed even if reading fails
		return;
	}
	file2.close(); // Close the file after reading

	// Add the read flowData to flowList
	flowList.push_back(tempFlow);

	// Print out the flowList
	for (const auto &flow : flowList)
	{
		ESP_LOGI(TAG, "Flow: %s", flow.name.c_str());
	}
}
void loop()
{
	vTaskDelete(NULL);
}
