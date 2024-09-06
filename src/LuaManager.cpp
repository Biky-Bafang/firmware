#include "LuaManager.h"
#include "SettingsManager.h"

#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include "lua/rtoslib.h"
#include "lua/esplib.h"

static const char *TAG = "LuaManager";

// Constructor
LuaManager::LuaManager() {}

TaskHandle_t LuaManager::taskHandle = NULL;
lua_State *L = luaL_newstate();

static void load_custom_libs(lua_State *L)
{
	luaL_requiref(L, "rtos", luaopen_lrtos, 1);
	luaL_requiref(L, "esp", luaopen_lesp, 1);
	lua_pop(L, 1);
}

// Initialize the Lua manager
void LuaManager::init(JsonDocument *settings)
{
	ESP_LOGI(TAG, "Initializing Lua");
	LuaParams *params = new LuaParams{*settings, this};
	xTaskCreate(
		LuaManager::loop, /* Task function. */
		"LuaManager",	  /* String with name of task. */
		10000,			  /* Stack size in bytes. */
		params,			  /* Parameter passed as input of the task */
		1,				  /* Priority of the task. */
		&taskHandle);	  /* Task handle. */
}

void LuaManager::loop(void *parameter)
{
	LuaParams *params = static_cast<LuaParams *>(parameter);
	JsonDocument *settings = &params->settings;

	luaL_openlibs(L);
	load_custom_libs(L);

	while (true)
	{
		luaL_dostring(L, "esp.logi(\"Loop\",\"Hello world from Lua!\")");
		delay(1000);
	}
	lua_close(L);
	vTaskDelete(NULL);
}
