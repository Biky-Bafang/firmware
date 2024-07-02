#include <Arduino.h>
#include "BLEManager.h"
#include "DebugManager.h"
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define RX0 13
#define TX0 12
#define RX1 17
#define TX1 18
#define RX2 16
#define TX2 15

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
				bleManager.sendData(settings);
			}
			if (value[0] == 0x02)
			{
				ESP.restart();
			}
			if (value[0] == 0x03)
			{
				settings.clear();
				EepromStream eepromStream(0, 512);
				serializeJson(settings, eepromStream);
				EEPROM.commit();
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
			JsonDocument tempDoc;

			while (1)
			{
				if (dataCache.size() > 0 && dataCache[0].containsKey("type"))
				{
					JsonDocument tempDoc;
					deserializeJson(tempDoc, dataCache[0]);
					responseDoc["value"] = tempDoc["value"];
					bleManager.sendData(responseDoc);
					break;
				}
				vTaskDelay(1);
			}
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
			serializeJson(dataCache, Serial);
			// create an variable json doc with convertToJson(dataCache[0])
			JsonDocument tempDoc;
			deserializeJson(tempDoc, dataCache[0]);
			bleManager.sendData(tempDoc);
			dataCache.remove(0);
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

void setup()
{
	Serial.begin(115200);
	Serial0.begin(115200, SERIAL_8N1, RX0, TX0);
	Serial1.begin(1200, SERIAL_8N1, RX1, TX1);
	Serial2.begin(1200, SERIAL_8N1, RX2, TX2);
	EEPROM.begin(512);
	// read the settings from EEPROM
	EepromStream eepromStream(0, 512);
	DeserializationError error = deserializeJson(settings, eepromStream);
	if (error || settings.size() <= 3)
	{
		settings["name"] = "Biky 12f";
		settings["password"] = "12345678";
		settings["packetDelay"] = 10;
		settings["txPower"] = 9;
	}
	bufferTimeDelay = settings["packetDelay"];
	bleManager.init(settings["name"], new MyCharacteristicCallbacks(), bleManager.powerLevel(settings["txPower"]));

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected to WiFi");
	Serial.println(WiFi.localIP());

	// print out all the settings as name: value
	for (JsonPair kv : settings.as<JsonObject>())
	{
		// send it to jtag
		esp_log_level_set("*", ESP_LOG_VERBOSE);
		ESP_LOGW("Settings", "%s: %s", kv.key().c_str(), kv.value().as<String>().c_str());
	}
	ArduinoOTA.setHostname(settings["name"].as<String>().c_str());
	ArduinoOTA.begin();

	// start the task
	xTaskCreate(serial2ToSerial1, "SerialToSerial1", 8000, NULL, 1, NULL);
	xTaskCreate(serial1ToSerial2, "Serial1ToSerial", 8000, NULL, 1, NULL);
	xTaskCreate(otaTask, "OTA", 8000, NULL, 1, NULL);
	xTaskCreate(cacheHandler, "CacheHandler", 8000, NULL, 1, NULL);
}

void loop()
{
}
