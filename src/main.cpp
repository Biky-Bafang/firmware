#include <Arduino.h>
#include "BLEManager.h"
#include "DebugManager.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <SD_MMC.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
#define REASSIGN_PINS

#define cRX1 9
#define cTX1 10
#define cRX2 12
#define cTX2 11

#define CLK 4
#define CMD 6
#define DATA0 5
#define CD 7
// #define CLK 4
// #define CMD 5
// #define DATA0 6
// #define CD 8

#define ssid "decop7"
#define password "!10057704a"

BLEManager bleManager;
DebugManager debugManager;
JsonDocument simpleDoc;
JsonArray dataCache = simpleDoc.to<JsonArray>();
JsonDocument settings;
int bufferTimeDelay = 10;
int serialDelay = 10;

struct PatternResponse
{

	const uint8_t *pattern;
	size_t patternSize;
	const uint8_t *response;
	size_t responseSize;
};
PatternResponse patterns[] = {
	{new uint8_t[5]{0x11, 0x51, 0x04, 0xb0, 0x05}, 5,
	 new uint8_t[19]{0x51, 0x10, 0x48, 0x5A, 0x58, 0x54, 0x53, 0x5A, 0x5A, 0x36, 0x32, 0x32, 0x32, 0x30, 0x31, 0x31, 0x01, 0x14, 0x1B}, 19},
	{new uint8_t[2]{0x11, 0x52}, 2,
	 new uint8_t[27]{0x52, 0x18, 0x1F, 0x0F, 0x00, 0x1C, 0x25, 0x2E, 0x37, 0x40, 0x49, 0x52, 0x5B, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x34, 0x01, 0xD}, 27},
	{new uint8_t[2]{0x11, 0x53}, 2,
	 new uint8_t[14]{0x53, 0x0B, 0x03, 0xFF, 0xFF, 0x64, 0x06, 0x14, 0x0A, 0x19, 0x08, 0x14, 0x14, 0x27}, 14},
	{new uint8_t[2]{0x11, 0x54}, 2,
	 new uint8_t[9]{0x54, 0x06, 0x0B, 0x23, 0x00, 0x03, 0x11, 0x14, 0xAC}, 9},

};

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
				String tempvalue;
				serializeJson(settings, tempvalue);
				dataCache.add(tempvalue);
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
				deserializeJson(responseDoc, "{\"sd\":[],\"core\":[{\"id\":\"Core-0\",\"icon\":\"faBluetoothB\",\"label\":\"Start\",\"nodes\":5,\"triggers\":1}]}");
				String tempvalue;
				serializeJson(responseDoc, tempvalue);
				dataCache.add(tempvalue);
				return;
			}
		}
		else if (value[0] == 0xf2)
		{
			value.erase(0, 1);
			if (value[0] == 0x01)
			{
				settings["name"] = value.substr(1);
			}
			if (value[0] == 0x02)
			{
				settings["password"] = value.substr(1);
			}
			if (value[0] == 0x03)
			{
				settings["packetDelay"] = int(value[1]);
			}
			if (value[0] == 0x04)
			{
				settings["txPower"] = int(value[1]);
			}
			if (value[0] == 0x05)
			{
				// check if invertSerial1 is 1 or 0
				settings["invertSerial1"] = value[1] == 0x01;
			}
			if (value[0] == 0x06)
			{
				// check if invertSerial2 is 1 or 0
				settings["invertSerial2"] = value[1] == 0x01;
			}
			// save tghe settings to EEPROM
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
				if (dataCache.size() > 0 && dataCache[0].containsKey("type"))
				{
					JsonDocument tempDoc;
					deserializeJson(tempDoc, dataCache[0]);
					responseDoc["value"] = tempDoc["value"];
					dataCache.add(responseDoc);
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
			String tempvalue;
			serializeJson(tempDoc, tempvalue);
			dataCache.add(tempvalue);
		}
	}

	void onRead(BLECharacteristic *pCharacteristic) override
	{
		// Handle read request
	}
};

// create an task to send the data to ble
void cacheHandler(void *parameter)
{
	while (1)
	{
		if (dataCache.size() > 0)
		{
			// serializeJson(dataCache, Serial);
			// create an variable json doc with convertToJson(dataCache[0])
			JsonDocument tempDoc;
			deserializeJson(tempDoc, dataCache[0]);
			dataCache.remove(0);
			if (!bleManager.deviceConnected)
				continue;

			bleManager.sendData(tempDoc);
		}
		vTaskDelay(1); // Yield to other tasks, letting the system breathe
	}
}

// create an task so when it recieves on Serial it will send it to Serial1
void serial2ToSerial1(void *parameter)
{
	uint8_t buffer[100];
	size_t bufferIndex = 0;
	bool usedPattern = false;
	unsigned long lastBuffer = millis();

	while (1)
	{
		if (Serial2.available())
		{
			int c = Serial2.read();
			if (c == -1)
				continue;
			Serial1.write(c);

			buffer[strlen((char *)buffer)] = c;
			lastBuffer = millis();
			buffer[bufferIndex++] = (uint8_t)c;
			if (bufferIndex >= sizeof(buffer))
			{
				bufferIndex--;
			}
		}
		if (millis() - lastBuffer > bufferTimeDelay && bufferIndex > 0)
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
			tempDoc["type"] = "serial2";
			tempDoc["value"] = bufferString;
			String tempvalue;
			serializeJson(tempDoc, tempvalue);
			dataCache.add(tempvalue);
			bufferIndex = 0;
			memset(buffer, 0, sizeof(buffer));
		}
		vTaskDelay(1); // Yield to other tasks
	}
}

void serial1ToSerial2(void *parameter)
{
	uint8_t buffer[100];
	size_t bufferIndex = 0;
	bool usedPattern = false;
	unsigned long lastBuffer = millis();

	while (1)
	{
		if (Serial1.available())
		{
			int c = Serial1.read();
			if (c == -1)
				continue;

			// if buffer is "0x16 0x1a" if it is send 0xf1 otherwise send c
			if (buffer[0] == 0x16 && buffer[1] == 0x1a)
			{
				Serial2.write(0xf1);
			}
			else
			{
				Serial2.write(c);
			}
			buffer[strlen((char *)buffer)] = c;
			lastBuffer = millis();
			buffer[bufferIndex++] = (uint8_t)c;
			if (bufferIndex >= sizeof(buffer))
			{
				bufferIndex--;
			}
		}
		if (millis() - lastBuffer > bufferTimeDelay && bufferIndex > 0)
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
			tempDoc["type"] = "serial1";
			tempDoc["value"] = bufferString;
			String tempvalue;
			serializeJson(tempDoc, tempvalue);
			dataCache.add(tempvalue);
			bufferIndex = 0;
			memset(buffer, 0, sizeof(buffer));
		}
		vTaskDelay(1); // Yield to other tasks
	}
}

void otaTask(void *parameter)
{
	while (1)
	{
		ArduinoOTA.handle();
		vTaskDelay(1); // Yield to other tasks
	}
}
String info = "";
void setup()
{
	delay(1000);

	Serial.begin(115200);
	// Serial0.begin(115200, SERIAL_8N1, RX0, TX0);
	EEPROM.begin(512);
	// read the settings from EEPROM
	EepromStream eepromStream(0, 512);
	DeserializationError error = deserializeJson(settings, eepromStream);
	if (error || settings.size() <= 5)
	{
		settings["name"] = "Biky 12f";
		settings["password"] = "12345678";
		settings["packetDelay"] = 10;
		settings["txPower"] = 9;
		settings["invertSerial1"] = false;
		settings["invertSerial2"] = false;
	}
	bufferTimeDelay = settings["packetDelay"];
	bleManager.init(settings["name"], new MyCharacteristicCallbacks(), bleManager.powerLevel(settings["txPower"]));

	// swap RX and tx if invertSerial1 is true
	Serial1.begin(1200, SERIAL_8N1, cRX1, cTX1, settings["invertSerial1"]);

	Serial2.begin(1200, SERIAL_8N1, cRX2, cTX2, settings["invertSerial2"]);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to WiFi");
	Serial.println(WiFi.localIP());

	// // ArduinoOTA.setHostname(settings["name"].as<String>().c_str());
	// // ArduinoOTA.begin();

	// start the task
	xTaskCreate(serial2ToSerial1, "SerialToSerial1", 8000, NULL, 1, NULL);
	xTaskCreate(serial1ToSerial2, "Serial1ToSerial", 8000, NULL, 1, NULL);
	// xTaskCreate(otaTask, "OTA", 8000, NULL, 1, NULL);
	xTaskCreate(cacheHandler, "CacheHandler", 8000, NULL, configMAX_PRIORITIES - 1, NULL);

	// set cs pin
	pinMode(CD, INPUT_PULLDOWN);
	// setup sd card and use the custom pins with using spi channel 3
	// pullup CMD and DATA0
	SD_MMC.setPins(CLK, CMD, DATA0);
}
bool sdInit = false;

void loop()
{
	delay(100);
	if (digitalRead(CD) == HIGH && !sdInit)
	{
		sdInit = true;
		settings["sdCard"] = true;
		// read it and serial log it
		SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 4);
		// print test.txt
		File file = SD_MMC.open("/test.txt");
		if (!file)
		{
			Serial.println("Failed to open file for reading");
			return;
		}
		Serial.println("File Content:");
		while (file.available())
		{
			Serial.write(file.read());
		}
		file.close();
		Serial.println();
	}
	else if (digitalRead(CD) == LOW && sdInit)
	{
		sdInit = false;
		settings["sdCard"] = false;
		SD_MMC.end();
	}
}
