#include "BLEManager.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *BLEManager::pServer = nullptr;
BLECharacteristic *BLEManager::pCharacteristic = nullptr;
BLECharacteristic *BLEManager::pCharacteristic_1 = nullptr;
bool BLEManager::deviceConnected = false;
JsonDocument simpleDoc;

BLEManager::BLEManager() : dataCache() {}

// create jsonToHexString
String BLEManager::jsonToHexString(JsonDocument &doc)
{
	String jsonString;
	serializeJson(doc, jsonString);
	String hexString = "ff";
	for (int i = 0; i < jsonString.length(); i++)
	{
		hexString += String(jsonString[i], HEX);
	}
	return hexString;
}

esp_power_level_t BLEManager::powerLevel(int powerLevel)
{
	switch (powerLevel)
	{
	case 3:
		return ESP_PWR_LVL_P3;
	case 6:
		return ESP_PWR_LVL_P6;
	case 9:
		return ESP_PWR_LVL_P9;
	case 12:
		return ESP_PWR_LVL_P12;
	case 15:
		return ESP_PWR_LVL_P15;
	case 18:
		return ESP_PWR_LVL_P18;
	case 21:
		return ESP_PWR_LVL_P21;
	default:
		return ESP_PWR_LVL_P9;
	}
}

void BLEManager::init(
	String deviceName,
	BLECharacteristicCallbacks *charCallbacks,
	esp_power_level_t powerLevel)
{
	dataCache = simpleDoc.to<JsonArray>();
	BLEDevice::init(deviceName.c_str());
	pServer = BLEDevice::createServer();
	BLEDevice::setPower(powerLevel);
	pServer->setCallbacks(new BLEManager());

	BLEService *pService = pServer->createService(SERVICE_UUID);
	pCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_NOTIFY);
	pCharacteristic->addDescriptor(new BLE2902());
	pService->start();
	// set callback to onWrite
	if (charCallbacks)
	{
		pCharacteristic->setCallbacks(charCallbacks); // Use provided characteristic callbacks
	}

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->setScanResponse(false);
	pAdvertising->setMinPreferred(0x0);
	BLEDevice::startAdvertising();
}

void BLEManager::stop()
{
	// turn off BLE
	BLEDevice::deinit(true);
}

void BLEManager::cacheHandler(void *parameter)
{
	BLEManager *instance = static_cast<BLEManager *>(parameter);

	while (1)
	{
		if (dataCache.size() > 0)
		{
			// serializeJson(dataCache, Serial);
			// create an variable json doc with convertToJson(dataCache[0])
			JsonDocument tempDoc;
			deserializeJson(tempDoc, dataCache[0]);
			dataCache.remove(0);
			if (!instance->deviceConnected)
				continue;

			instance->sendData(tempDoc, false);
		}
		vTaskDelay(1); // Yield to other tasks, letting the system breathe
	}
}

void BLEManager::sendData(JsonDocument &doc)
{
	sendData(doc, true);
}
void BLEManager::sendData(JsonDocument &doc, bool cached)
{
	if (cached)
	{
		String tempvalue;
		serializeJson(doc, tempvalue);
		dataCache.add(tempvalue);
		return;
	}
	if (deviceConnected)
	{
		String hexString = jsonToHexString(doc);
		size_t jsonLength = hexString.length() / 2; // Each byte is represented by two hex characters
		size_t offset = 0;

		while (offset < jsonLength)
		{
			size_t chunkSize = min(jsonLength - offset, (size_t)20); // BLE typical MTU is 20 bytes
			uint8_t chunk[20];
			for (size_t i = 0; i < chunkSize; i++)
			{
				char hexByte[3];
				hexByte[0] = hexString[offset * 2];
				hexByte[1] = hexString[offset * 2 + 1];
				hexByte[2] = '\0';
				chunk[i] = strtol(hexByte, NULL, 16);
				offset++;
			}
			pCharacteristic->setValue(chunk, chunkSize);
			pCharacteristic->notify();
			delay(10); // Small delay to prevent BLE stack congestion
		}
	}
}

void BLEManager::onConnect(BLEServer *pServer)
{
	deviceConnected = true;
}

void BLEManager::onDisconnect(BLEServer *pServer)
{
	deviceConnected = false;
	// start advertising
	BLEDevice::startAdvertising();
}
