#include "seriallib.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "lserial";
static int lwrite(lua_State *L)
{
	// get the first argument as string
	const char *device = luaL_checkstring(L, 1);
	// check if device is either "motor" or "display"
	if (strcmp(device, "motor") != 0 && strcmp(device, "display") != 0)
	{
		luaL_error(L, "Invalid device");
		return 0;
	}
	size_t len;
	const char *data = luaL_checklstring(L, 2, &len);

	if (len == 0)
	{
		luaL_error(L, "Data is empty");
		return 0;
	}

	// serial write to Serial1 if device is motor or Serial2 if device is display
	if (strcmp(device, "motor") == 0)
	{
		Serial1.write(data, len);
	}
	else
	{
		Serial2.write(data, len);
	}
	return 0;
}

static const struct luaL_Reg lserial_funcs[] = {
	{"write", lwrite},
	{NULL, NULL}};

int luaopen_lserial(lua_State *L)
{
	luaL_newlib(L, lserial_funcs);
	return 1;
}