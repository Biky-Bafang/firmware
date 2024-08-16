#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
class BLEManager : public BLEServerCallbacks
{
public:
	BLEManager();
	static BLEManager &getInstance()
	{
		static BLEManager instance;
		return instance;
	}

	BLEManager(BLEManager const &) = delete;
	void operator=(BLEManager const &) = delete;

	void init(
		String deviceName,
		BLECharacteristicCallbacks *charCallbacks,
		esp_power_level_t powerLevel = ESP_PWR_LVL_P9);
	void stop();
	void sendData(JsonDocument &doc); // Pass by reference
	void sendData(JsonDocument &doc, bool cached);
	void requestSettingsUpdate();
	static bool deviceConnected;
	String jsonToHexString(JsonDocument &doc);
	esp_power_level_t powerLevel(int powerLevel);
	JsonArray &getDataCache() { return dataCache; }
	bool &getDeviceConnected() { return deviceConnected; }
	static void cacheHandler(void *parameter);

	JsonArray dataCache;
	int bleStatus = 0;

private:
	static BLEServer *pServer;
	static BLECharacteristic *pCharacteristic;
	static BLECharacteristic *pCharacteristic_1;

	void onConnect(BLEServer *pServer) override;
	void onDisconnect(BLEServer *pServer) override;

	SemaphoreHandle_t dataCacheMutex;
};

#endif // BLE_MANAGER_H
