#include "SettingsManager.h"

// Constructor
SettingsManager::SettingsManager() {}

static const char *TAG = "SettingsManager";

void SettingsManager::init(JsonDocument *settings, std::vector<flowData> *flowList)
{
	ESP_LOGI(TAG, "Initializing Settings");
	EEPROM.begin(512);
	EepromStream eepromStream(0, 512);
	DeserializationError error = deserializeJson(*settings, eepromStream);
	if (error || (*settings).size() < 8)
	{
		// check if (*settings)["name"] exists and is not empty
		(*settings)["name"] = (*settings)["name"] | "Biky";
		(*settings)["ssid"] = (*settings)["ssid"] | "changeme";
		(*settings)["password"] = (*settings)["password"] | "changeme";
		(*settings)["packetDelay"] = (*settings)["packetDelay"] | 10;
		(*settings)["txPower"] = (*settings)["txPower"] | 9;
		(*settings)["invertSerial1"] = (*settings)["invertSerial1"] | false;
		(*settings)["invertSerial2"] = (*settings)["invertSerial2"] | false;
		(*settings)["powerLock"] = (*settings)["powerLock"] | false;
	}
	(*settings)["firmwareVersion"] = firmwareVersion;
	(*settings)["hardwareVersion"] = hardwareVersion;

	xTaskCreate(
		SettingsManager::loop, /* Task function. */
		"SettingsManager",	   /* String with name of task. */
		10000,				   /* Stack size in bytes. */
		this,				   /* Parameter passed as input of the task */
		1,					   /* Priority of the task. */
		NULL);				   /* Task handle. */
}
std::string SettingsManager::readString(fs::File &file)
{
	uint16_t length;
	if (file.read((uint8_t *)&length, sizeof(length)) != sizeof(length))
	{
		ESP_LOGE(TAG, "Failed to read string length");
		return "";
	}

	std::vector<char> buffer(length + 1);
	if (file.read((uint8_t *)buffer.data(), length) != length)
	{
		ESP_LOGE(TAG, "Failed to read string data");
		return "";
	}

	buffer[length] = '\0'; // Null-terminate the string
	return std::string(buffer.data());
}

void SettingsManager::writeString(fs::File &file, const std::string &str)
{
	uint16_t length = str.size(); // Store the length of the string first (2 bytes)
	if (file.write((uint8_t *)&length, sizeof(length)) != sizeof(length))
	{
		ESP_LOGE(TAG, "Failed to write string length");
		return;
	}
	if (file.write((uint8_t *)str.c_str(), length) != length)
	{
		ESP_LOGE(TAG, "Failed to write string data");
	}
}

bool SettingsManager::dumpFlowDataToBinary(fs::File &file, const flowData &flow)
{
	writeString(file, flow.name);
	writeString(file, flow.id);
	writeString(file, flow.trigger_type);
	writeString(file, flow.trigger_data);
	writeString(file, flow.trigger_device);
	writeString(file, flow.lua_code);

	uint16_t numVariables = flow.variables.size();
	if (file.write((uint8_t *)&numVariables, sizeof(numVariables)) != sizeof(numVariables))
	{
		ESP_LOGE(TAG, "Failed to write number of variables");
		return false;
	}

	for (const auto &variable : flow.variables)
	{
		writeString(file, variable.name);
		writeString(file, variable.type);
		writeString(file, variable.value);

		uint8_t persistByte = variable.persist ? 1 : 0;
		if (file.write((uint8_t *)&persistByte, sizeof(persistByte)) != sizeof(persistByte))
		{
			ESP_LOGE(TAG, "Failed to write persist flag");
			return false;
		}
	}

	uint8_t coreFlowByte = flow.coreFlow ? 1 : 0;
	if (file.write((uint8_t *)&coreFlowByte, sizeof(coreFlowByte)) != sizeof(coreFlowByte))
	{
		ESP_LOGE(TAG, "Failed to write coreFlow");
	}
	ESP_LOGI(TAG, "Flow data written to file");
	return true;
}

bool SettingsManager::readFlowDataFromBinary(fs::File &file, flowData &flow)
{

	flow.name = readString(file);
	ESP_LOGI(TAG, "Name: %s", flow.name.c_str());

	flow.id = readString(file);
	ESP_LOGI(TAG, "ID: %s", flow.id.c_str());

	flow.trigger_type = readString(file);
	ESP_LOGI(TAG, "Trigger Type: %s", flow.trigger_type.c_str());

	flow.trigger_data = readString(file);
	ESP_LOGI(TAG, "Trigger Data: %s", flow.trigger_data.c_str());

	flow.trigger_device = readString(file);
	ESP_LOGI(TAG, "Trigger Device: %s", flow.trigger_device.c_str());

	flow.lua_code = readString(file);
	ESP_LOGI(TAG, "Lua Code: %s", flow.lua_code.c_str());

	uint16_t numVariables;
	if (file.read((uint8_t *)&numVariables, sizeof(numVariables)) != sizeof(numVariables))
	{
		ESP_LOGE(TAG, "Failed to read number of variables");
		return false;
	}

	for (uint16_t i = 0; i < numVariables; ++i)
	{
		Variable var;
		var.name = readString(file);
		var.type = readString(file);
		var.value = readString(file);

		uint8_t persistByte;
		if (file.read((uint8_t *)&persistByte, sizeof(persistByte)) != sizeof(persistByte))
		{
			ESP_LOGE(TAG, "Failed to read persist flag");
			return false;
		}
		var.persist = (persistByte == 1);

		ESP_LOGI(TAG, "Variable: %s, %s, %s, %d", var.name.c_str(), var.type.c_str(), var.value.c_str(), var.persist);
		flow.variables.push_back(var);
	}

	uint8_t coreFlowByte;
	if (file.read((uint8_t *)&coreFlowByte, sizeof(coreFlowByte)) != sizeof(coreFlowByte))
	{
		ESP_LOGE(TAG, "Failed to read coreFlow");
		return false;
	}
	flow.coreFlow = (coreFlowByte == 1);

	ESP_LOGI(TAG, "Core Flow: %d", flow.coreFlow);
	return true;
}

void SettingsManager::loop(void *parameter)
{
	while (true)
	{
		delay(1000);
	}
}