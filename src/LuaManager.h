#ifndef LUAMANAGER_H // Header guard start
#define LUAMANAGER_H

#include <Arduino.h>
#include <RTOS.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>

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

	void init(JsonDocument *settings);

private:
	static void loop(void *parameter);

	struct LuaParams
	{
		JsonDocument &settings;
		LuaManager *instance; // Add instance pointer for accessing non-static members
	};
	static TaskHandle_t taskHandle;
};

#endif // Header guard end
