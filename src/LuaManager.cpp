#include "LuaManager.h"
#include "SettingsManager.h"

#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#include "lua/rtoslib.h"
#include "lua/esplib.h"
#include "lua/seriallib.h"

static const char *TAG = "LuaManager";

// Constructor
LuaManager::LuaManager() {}

static int lua_mem_size = 0;
static void *l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	int new_size = lua_mem_size - osize + nsize;
	if (new_size > LUA_MAX_MEMSIZE)
	{
		ESP_LOGE(TAG, "Error! Lua wants more memory than we can allocate: %u > %u\n", new_size, LUA_MAX_MEMSIZE);
		// restart lua instance
		return NULL;
	}
	// ESP_LOGI(TAG, "Reallocating: old = %u, new = %u\n", lua_mem_size, new_size);
	lua_mem_size = new_size;

	if (nsize == 0)
	{
		free(ptr);
		return NULL;
	}
	else
		return realloc(ptr, nsize);
}
lua_State *L = lua_newstate(l_alloc, NULL);
TaskHandle_t LuaManager::taskHandle = NULL;

static void report(lua_State *L, int status)
{
	if (status == LUA_OK)
		return;

	const char *msg = lua_tostring(L, -1);
	ESP_LOGE(TAG, "Error: %s\n", msg);
	lua_pop(L, 1);
}

static void load_custom_libs(lua_State *L)
{
	luaL_requiref(L, "rtos", luaopen_lrtos, 1);
	luaL_requiref(L, "esp", luaopen_lesp, 1);
	luaL_requiref(L, "serial", luaopen_lserial, 1);
	lua_pop(L, 1);
}

// Initialize the Lua manager
void LuaManager::init(std::vector<flowData> *flowList)
{
	ESP_LOGI(TAG, "Initializing Lua");
	LuaParams *params = new LuaParams{*flowList, this};

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
	std::vector<flowData> *flowList = &params->flowList;
	vTaskDelete(NULL);
	if (!L)
	{
		ESP_LOGE(TAG, "Could not create state\n");
		while (1)
		{
			vTaskDelay(1000);
		}
	}

	luaL_openlibs(L);
	load_custom_libs(L);
	while (true)
	{
		delay(1000);
	}
	lua_close(L);
	vTaskDelete(NULL);
}

Variable LuaManager::run(flowData flow)
{
	Variable var;
	if (!L)
	{
		ESP_LOGE(TAG, "Could not create state\n");
		return var;
	}

	luaL_openlibs(L);
	load_custom_libs(L);
	// add the variables to the lua state
	for (auto const &variable : flow.variables)
	{
		lua_pushstring(L, variable.value.c_str());
		lua_setglobal(L, variable.name.c_str());
	}
	// run the lua code
	int r = luaL_dostring(L, flow.lua_code.c_str());
	if (r)
	{
		report(L, r);
		return var;
	}
	lua_pop(L, 1);
	r = lua_pcall(L, 0, 0, 0);
	return var;
}