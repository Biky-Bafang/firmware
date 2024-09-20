#ifndef LUAMANAGER_H // Header guard start
#define LUAMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include "settingsManager.h"
#define LUA_MAX_MEMSIZE 0x10000 // 64k

extern "C"
{
#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}

// Class definition

class LuaManager // Class declaration
{
public:
	LuaManager(); // Constructor declaration

	static LuaManager &getInstance()
	{
		static LuaManager instance;
		return instance;
	}

	LuaManager(LuaManager const &) = delete;
	void operator=(LuaManager const &) = delete;

	void init(std::vector<flowData> *flowList);
	// make an run function with the flowData as parameter and it should return the new variables
	Variable run(flowData flow);

private:
	static void loop(void *parameter);

	struct LuaParams
	{
		std::vector<flowData> &flowList;
		LuaManager *instance; // Add instance pointer for accessing non-static members
	};
	static TaskHandle_t taskHandle;
};

#endif // Header guard end
