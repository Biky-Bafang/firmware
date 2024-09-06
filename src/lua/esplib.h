#ifndef ESPLIB_H
#define ESPLIB_H

#include <stdio.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua/lua.h>
#include <lua/lauxlib.h>
int luaopen_lesp(lua_State *L);

#ifdef __cplusplus
}
#endif


#endif /* MAIN_RTOSLIB_H_ */
