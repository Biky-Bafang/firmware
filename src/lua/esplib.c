#include "esplib.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Arduino.h>
#include <cJSON.h>

static int lmillis(lua_State *L)
{
	lua_pushinteger(L, millis());
	return 1;
}
static int lmicros(lua_State *L)
{
	lua_pushinteger(L, micros());
	return 1;
}
static const char *TAG = "lesp";
static int llogi(lua_State *L)
{
	const char *tag = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	ESP_LOGI(tag, "%s", msg);
	return 0;
}

static int lloge(lua_State *L)
{
	const char *tag = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	ESP_LOGE(tag, "%s", msg);
	return 0;
}

static int llogw(lua_State *L)
{
	const char *tag = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	ESP_LOGW(tag, "%s", msg);
	return 0;
}
static int llogd(lua_State *L)
{
	const char *tag = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	ESP_LOGD(tag, "%s", msg);
	return 0;
}
static int llogv(lua_State *L)
{
	const char *tag = luaL_checkstring(L, 1);
	const char *msg = luaL_checkstring(L, 2);
	ESP_LOGV(tag, "%s", msg);
	return 0;
}

static const struct luaL_Reg lesp_funcs[] = {
	{"millis", lmillis},
	{"micros", lmicros},
	{"logi", llogi},
	{"loge", lloge},
	{"logw", llogw},
	{"logd", llogd},
	{"logv", llogv},
	{NULL, NULL}};

int luaopen_lesp(lua_State *L)
{
	luaL_newlib(L, lesp_funcs);
	return 1;
}