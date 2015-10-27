#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <assert.h> 
#include <stdarg.h>

#include <lua.h>  
#include <lualib.h>  
#include <lauxlib.h>  
#include "lib_commit.h"
#include "commit_common.h"
#include "wireless.h"
#include "dump.c"


#define CHK(con, msg) do{if(!(con)) {fprintf(stderr, "%s %d %s\n", __FILE__, __LINE__, msg); exit(-1);}}while(0)

static struct ap_cfg_info_st *set_basic(lua_State *L) {
	LOG_DEBUG("set basic start...");
	lua_getfield(L, 1, "basic");  
	luaL_argcheck(L, lua_istable(L, -1), 1, "missing basic");
	
	int ret; 
	struct ap_cfg_info_st *obj = create_config_obj(CONF_BASIC); 	CHK(obj, "create_config_obj fail");
	lua_getfield(L, -1, "code");
	int32_t code = lua_tointeger(L, -1);
	luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing code");
	ret = set_basic_country_code(obj, code); 		CHK(!ret, "set_basic_country_code fail");
	lua_pop(L, 1);
	
	lua_getfield(L, -1, "mode");
	int8_t mode = lua_tointeger(L, -1);
	luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing mode");
	ret = set_basic_work_mode(obj, mode); 		CHK(!ret, "set_basic_work_mode fail");
	lua_pop(L, 1); 

    lua_getfield(L, -1, "debug");
	uint8_t debug = lua_tointeger(L, -1);
	luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing debug sw");
	ret = set_basic_debug_sw(obj, debug); 		CHK(!ret, "set debug_sw fail");
	lua_pop(L, 1); 
	
	lua_pop(L, 1); 
	LOG_DEBUG("set basic end.");
	return obj;
}

static struct ap_cfg_info_st *set_radio(lua_State *L) {
	LOG_DEBUG("set radio start...");
	lua_getfield(L, 1, "radio");  
	luaL_argcheck(L, lua_istable(L, -1), 1, "missing radio");

	int ret;
	struct ap_cfg_info_st *radio = NULL;
	
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		struct ap_cfg_info_st *obj = create_config_obj(CONF_RADIO);		CHK(obj, "create_config_obj fail");
		int idx = lua_tointeger(L, -2);
		luaL_argcheck(L, lua_istable(L, -1), -1, "invalid radio array");
		lua_getfield(L, -1, "type");
		int8_t type = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing type");
		ret = set_radio_rf_type(obj, type); 		CHK(!ret, "set_radio_rf_type fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "enable");
		int8_t enable = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing enable");
		ret = set_radio_rf_enable(obj, enable); 		CHK(!ret, "set_radio_rf_enable fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "mode");
		const char *mode = lua_tostring(L, -1);
		luaL_argcheck(L, lua_isstring(L, -1), -1, "missing mode");
		ret = set_radio_rf_mode(obj, mode); 		CHK(!ret, "set_radio_rf_mode fail");
		lua_pop(L, 1);
		lua_getfield(L, -1, "bandwidth");
		int8_t bandwidth = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing bandwidth");
		ret = set_radio_channel_width(obj, bandwidth); 		CHK(!ret, "set_radio_channel_width fail");
		lua_pop(L, 1);
		lua_getfield(L, -1, "channel");
		int8_t channel = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing channel");
		ret = set_radio_channel_id(obj, channel); 		CHK(!ret, "set_radio_channel_id fail");
		lua_pop(L, 1);
		lua_getfield(L, -1, "power");
		int8_t power = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing power");
		ret = set_radio_power(obj, power); 		CHK(!ret, "set_radio_power fail");
		lua_pop(L, 1);
		lua_getfield(L, -1, "uplimit");
		int32_t uplimit = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing uplimit");
		ret = set_radio_users_uplimit(obj, uplimit); 		CHK(!ret, "set_radio_users_uplimit fail");
		lua_pop(L, 1);

		lua_getfield(L, -1, "vap_bridge");
		int32_t bridge = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing vap bridge");
		ret = set_optim_bridge(obj, bridge); 	CHK(!ret, "set_optim_bridge fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "mult_inspeed");
		int8_t inspeed = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing mult_inspeed");
		ret = set_optim_mult_inspeed(obj, inspeed); 	CHK(!ret, "set_optim_mult_inspeed fail");
		lua_pop(L, 1); 

		lua_getfield(L, -1, "sta_rate_limit");
		int8_t rate_limit = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing sta_rate_limit");
		ret = set_optim_sta_rate_limit(obj, rate_limit); 	CHK(!ret, "set_optim_sta_rate_limit fail");
		lua_pop(L, 1); 

		lua_getfield(L, -1, "mult_optim_enable");
		int8_t mult_optim = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing mult_optim_enable");
		ret = set_optim_mult_optim(obj, mult_optim); 		CHK(!ret, "set_optim_mult_optim fail");
		lua_pop(L, 1); 

		lua_getfield(L, -1, "fairtime_enable");
		int8_t fairtime = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing fairtime_enable");
		ret = set_optim_fairtime(obj, fairtime); 		CHK(!ret, "set_optim_fairtime fail");
		lua_pop(L, 1); 
	
		lua_pop(L, 1); 
		if (!radio) {
			radio = obj;
		} else {
			radio = pack_config_objs(2, radio, obj);
		}
	}
	
	lua_pop(L, 1);
	LOG_DEBUG("set radio end.");
	return radio;
}

static struct ap_cfg_info_st *set_wlan(lua_State *L) {
	LOG_DEBUG("set wlan start...");
	lua_getfield(L, 1, "wlan");  
	luaL_argcheck(L, lua_istable(L, -1), 1, "missing wlan");
	
	int ret; 
	struct ap_cfg_info_st *wlan = NULL;
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		struct ap_cfg_info_st *obj = create_config_obj(CONF_WLAN);		CHK(obj, "create_config_obj fail");
		
		int idx = lua_tointeger(L, -2);
		luaL_argcheck(L, lua_istable(L, -1), -1, "invalid wlan array");
		
		lua_getfield(L, -1, "type");
		int8_t type = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing type");
		ret = set_wlan_rf_type(obj, type); 		CHK(!ret, "set_wlan_rf_type fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "enable");
		int8_t enable = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing enable");
		ret = set_wlan_enable(obj, enable); 		CHK(!ret, "set_wlan_enable fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "wlanid");
		int16_t wlanid = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing wlanid");
		ret = set_wlan_id(obj, wlanid); 		CHK(!ret, "set_wlan_id fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "hide");
		int8_t hide = lua_tointeger(L, -1);
		luaL_argcheck(L, lua_isnumber(L, -1), -1, "missing hide");
		ret = set_wlan_hide_ssid(obj, hide); 		CHK(!ret, "set_wlan_hide_ssid fail");
		lua_pop(L, 1);
		
		lua_getfield(L, -1, "ssid");
		const char *ssid = lua_tostring(L, -1);
		luaL_argcheck(L, lua_isstring(L, -1), -1, "missing ssid");
		ret = set_wlan_ssid(obj, ssid); 		CHK(!ret, "set_wlan_ssid fail");
		lua_pop(L, 1);		
		lua_pop(L, 1);
		
		if (!wlan) {
			wlan = obj;
		} else {
			wlan = pack_config_objs(2, wlan, obj);
		}
	}
	
	lua_pop(L, 1);
	LOG_DEBUG("set wlan end.");
	return wlan;
}	

static int l_commit(lua_State *L) {
	luaL_argcheck(L, lua_istable(L, 1), 1, "commit a table!");
	struct ap_cfg_info_st *basic = NULL, *radio = NULL, *wlan = NULL, *info = NULL;
	
	if ((basic = set_basic(L)) == NULL)
	{
		LOG_WARN("set basic failed.");
		goto FAILED;	
	}
	
	if ((radio = set_radio(L)) == NULL)
	{
		LOG_WARN("set radio failed.");
		goto FAILED;
	}

	wlan = set_wlan(L);	/*存在无wlan的情形*/
	if (wlan) 
	{
		info = pack_config_objs(3, basic, radio, wlan);
	} 
	else 
	{
		info = pack_config_objs(2, basic, radio);		
	}
	if (info == NULL)
	{
		goto FAILED;
	}
	//display_config_objs(info);
	config_deliver(info);
	free_config_obj(info);
	return 0;
FAILED:
	SAFE_FREE(basic);
	SAFE_FREE(radio);
	SAFE_FREE(wlan);
	return 0;
}
static lua_State *s_L = NULL;
int lua_print(const char *fmt, ...) {
	int ret;
	static char buff[4096];
	va_list argptr;
	
	lua_State *L = s_L;
	
	va_start(argptr, fmt);
	ret = vsnprintf(buff, sizeof(buff), fmt, argptr);
	buff[ret] = 0;
	va_end(argptr); 
	
	lua_getglobal(L, "lua_print_callback");
	if (!lua_isfunction(L, -1)) {
		fprintf(stdout, "%s\n", buff);
		lua_pop(L, 1);
	} else {
		lua_pushstring(L, buff);
		lua_pcall(L, 1, 0, 0);
	} 
	return 0;
}


static luaL_Reg reg[] = {
	{ "commit", 				l_commit},
	{ NULL, 					NULL }
};
 
LUALIB_API int luaopen_apcommit(lua_State *L) { 
	s_L = L;
	wl_set_log_cb(lua_print); 
	wirless_commit_init();
	luaL_register(L, "apcommit", reg);
	
	return 1;
}
