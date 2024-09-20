#ifndef SERIALLIB_H
#define SERIALLIB_H

#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <Arduino.h>
#include <HardwareSerial.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <lua/lua.h>
#include <lua/lauxlib.h>
	int luaopen_lserial(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_RTOSLIB_H_ */
