#ifndef BLE_MANAGER_H
#define BLEMANAGER_H

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
	void init(
		String deviceName,
		BLECharacteristicCallbacks *charCallbacks,
		esp_power_level_t powerLevel = ESP_PWR_LVL_P9);
	void stop();
	void sendData(JsonDocument &doc); // Pass by reference
	static bool deviceConnected;
	String jsonToHexString(JsonDocument &doc);
	esp_power_level_t powerLevel(int powerLevel);

private:
	static BLEServer *pServer;
	static BLECharacteristic *pCharacteristic;
	static BLECharacteristic *pCharacteristic_1;
	void onConnect(BLEServer *pServer) override;
	void onDisconnect(BLEServer *pServer) override;
};

#endif // BLE_MANAGER_H
