
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <assert.h> 

#include <lua.h>  
#include <lualib.h>  
#include <lauxlib.h>  
#include "ap_ioctl.h"
#include "dump.c"

int sock = -1;

typedef int (*method_func)(lua_State *L, void *param); 
typedef struct {
	const char *method;
	method_func mode_func;
	void *		ioctl_func;
} mode_ioctl_st;

#define CHK_STR(L, n) do{\
		if (!lua_isstring((L), (n))) {\
			lua_pushnil(L);\
			lua_pushfstring(L, "%s %d param %d should be string", __FILE__, __LINE__, (n));\
			return 2;\
		}\
	}while(0)

#define CHK_NUM(L, n) do{\
		if (!lua_isnumber((L), (n))) {\
			lua_pushnil(L);\
			lua_pushfstring(L, "%s %d param %d should be number", __FILE__, __LINE__, (n));\
			return 2;\
		}\
	}while(0)	
	
#define CHK_TAB(L, n) do{\
		if (!lua_istable((L), (n))) {\
			lua_pushnil(L);\
			lua_pushfstring(L, "%s %d param %d should be table", __FILE__, __LINE__, (n));\
			return 2;\
		}\
	}while(0)	
	
#define CHK_RET(ret, mode_ioctl) do{\
		if ((ret)) {\
			lua_pushnil(L); \
			lua_pushfstring(L, "%s %d %s fail %s", __FILE__, __LINE__, (mode_ioctl)->method, strerror((ret)));\
			return 2;\
		} \
		lua_pushboolean(L, 1);\
		return 1;\
	}while(0)

static int set_str_int_str(lua_State *L, void *param) {
	CHK_STR(L, 2);CHK_NUM(L, 3);CHK_STR(L, 4);

	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	assert(mode_ioctl);
	
	typedef int (*m_s_i_i)(const char *, int, const char *);
	m_s_i_i func = (m_s_i_i)mode_ioctl->ioctl_func;
	
	int ret = func(lua_tostring(L, 2), lua_tointeger(L, 3), lua_tostring(L, 4));
	CHK_RET(ret, mode_ioctl);
}

static int set_str_int_int(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_NUM(L, 3); CHK_NUM(L, 4);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_i_i)(int, char *, int, int);
	m_s_i_i func = (m_s_i_i)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
	CHK_RET(ret, mode_ioctl);
}

static int get_str_int(lua_State *L, void *param) {
	CHK_STR(L, 2);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	  

	assert(mode_ioctl && mode_ioctl->ioctl_func);
	
	typedef int (*m_s_ri)(int, char *, int *);
	m_s_ri func = (m_s_ri)mode_ioctl->ioctl_func; 	assert(func);

	int val = 0;
	//fprintf(stderr, "%s %d %d %s %s\n", __FILE__, __LINE__, sock, lua_tostring(L, 2), mode_ioctl->method);
	int ret = func(sock, lua_tostring(L, 2), &val);
	if (ret) {
		lua_pushnil(L);
		lua_pushfstring(L, "%s fail %s", mode_ioctl->method, strerror(ret));
		return 2;
	} 
	
	lua_pushinteger(L, val);
	return 1;
}

static int set_str_str(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_STR(L, 3);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_s)(int, char *, char *);
	m_s_s func = (m_s_s)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2), lua_tostring(L, 3));
	CHK_RET(ret, mode_ioctl);
}

static int set_str_str_int(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_STR(L, 3); CHK_NUM(L, 4);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	
	assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_s_i)(int, char *, char *, int);
	m_s_s_i func = (m_s_s_i)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2), lua_tostring(L, 3), lua_tointeger(L, 4));
	CHK_RET(ret, mode_ioctl);
}

static int set_str(lua_State *L, void *param) {
	CHK_STR(L, 2);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	
	assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s)(int, char *);
	m_s func = (m_s)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2));
	CHK_RET(ret, mode_ioctl);
}

static int set_str_int(lua_State *L, void *param) {
	CHK_STR(L, 2);CHK_NUM(L, 3);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_i)(int, char *, int);
	m_s_i func = (m_s_i)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2), lua_tointeger(L, 3));
	CHK_RET(ret, mode_ioctl);
}

static int set_str_str_str(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_STR(L, 3); CHK_STR(L, 4);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_s_i)(int, char *, char *, char *);
	m_s_s_i func = (m_s_s_i)mode_ioctl->ioctl_func; 	assert(func);

	int ret = func(sock, lua_tostring(L, 2), lua_tostring(L, 3), lua_tostring(L, 4));
	CHK_RET(ret, mode_ioctl);
}

static int set_str_int_p(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_NUM(L, 3); CHK_STR(L, 4);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);

	typedef int (*m_s_i_s_i)(int, char *, int, char *, int);
	m_s_i_s_i func = (m_s_i_s_i)mode_ioctl->ioctl_func; 	assert(func);

	size_t len;
	char *data;
	data = lua_tolstring(L, 4, &len);
	if (!data) {
		lua_pushnil(L);
		lua_pushfstring(L, "invalid param 4");
		return 2;	
	}
	int ret = func(sock, lua_tostring(L, 2), lua_tointeger(L, 3), data, len);
	CHK_RET(ret, mode_ioctl);
}

static int get_str_str(lua_State *L, void *param) {
	CHK_STR(L, 2); 
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);
	typedef int (*m_s_rs_i)(int, char *, char *, int);
	m_s_rs_i func = (m_s_rs_i)mode_ioctl->ioctl_func; 	assert(func);
	int max_size = 100 * 1024;
	char *buff = (char *)malloc(max_size);				
	if (!buff)	{fprintf(stderr, "malloc fail\n"); exit(-1);}
	int ret = func(sock, lua_tostring(L, 2), buff, max_size);
	if (ret) {
		free(buff);
		lua_pushnil(L); 
		lua_pushfstring(L, "%s %d %s fail %s", __FILE__, __LINE__, (mode_ioctl)->method, strerror((ret)));
		return 2;
	}
	lua_pushstring(L, buff);
	free(buff);
	return 1;
}

static int l_set_radio_info(lua_State *L, void *param) {
	CHK_STR(L, 2);  CHK_STR(L, 3); 	CHK_STR(L, 4); 
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);
	
	int i;
	size_t len;
	const char *str, *band, *ifname;
	unsigned int tmp[6] = {0}; 
	ifname = lua_tostring(L, 2);
	str = lua_tolstring(L, 3, &len);
	
	if (len != sizeof("00ffeeff00ff") - 1) {
		lua_pushnil(L); 
		lua_pushfstring(L, "invalid str %s", str);
		return 2;
	}

	band = lua_tostring(L, 4);
	if (!(!strncmp(band, "2g", 2) || !strncmp(band, "5g", 2))) {
		lua_pushnil(L); 
		lua_pushfstring(L, "invalid band %s", band);
		return 2;
	}
	
	ugw_ext_radio_info_t st;
	st.ap_devid = strtoull(str, NULL, 16);
	st.rf_type = (!strncmp(band, "2g", 2)) ? 0 : 1;
	
	int ret = set_radio_info(sock, ifname, &st);
	CHK_RET(ret, mode_ioctl); 
}

/* ifname, wlanid */
static int l_set_vap_info(lua_State *L, void *param) {
	CHK_STR(L, 2);  CHK_NUM(L, 3); 
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);
	
	ugw_ext_vap_info_t st;
	const char *ifname = lua_tostring(L, 2);
	st.wlan_id = (u_int32_t)lua_tointeger(L, 3);
	
	int ret = set_vap_info(sock, ifname, &st);
	CHK_RET(ret, mode_ioctl); 
}

/* mode,channel_arr,scantime */
static int l_start_scan(lua_State *L, void *param) {
	CHK_STR(L, 2); CHK_NUM(L, 3);	CHK_NUM(L, 4); CHK_TAB(L, 5);
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);
	
	ugw_ext_scan_params_t st;
	st.scan_mode = lua_tointeger(L, 3);	assert(st.scan_mode == 1 || st.scan_mode == 2);
	
	int i = 0;
	lua_pushnil(L);
	while (i < MAX_SCAN_CHAN_CNT && lua_next(L, -2)) {
		CHK_NUM(L, -1);
		st.scan_channel_list[i] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		i++;
	}
	
	if (i == 0) {
		lua_pushboolean(L, 1);
		return 1;
	}
	
	st.channel_num = i;	 
	st.scan_per_channel_time = lua_tointeger(L, 4);
	
	int ret = start_scan(sock, lua_tostring(L, 2), &st);
	CHK_RET(ret, mode_ioctl); 
}

/* ifname, wlanid */
static int l_vap_add_acl_mac(lua_State *L, void *param) {
	CHK_STR(L, 2);  CHK_STR(L, 3);  CHK_NUM(L, 4); 
	mode_ioctl_st *mode_ioctl = (mode_ioctl_st *)param;	 assert(mode_ioctl && mode_ioctl->ioctl_func);
	
	ugw_sta_acl_mac_t st;
	
	const char *ifname = lua_tostring(L, 2);
	st.seconds = (u_int32_t)lua_tointeger(L, 4);
	
	// TODO unsigned char ?
	const char *mac = lua_tostring(L, 3);
	int ret = sscanf(mac, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", \
				&st.mac[0], &st.mac[1], &st.mac[2], &st.mac[3], &st.mac[4], &st.mac[5]);
	if (ret != 6) {
		lua_pushnil(L); 
		lua_pushfstring(L, "invalid mac %s", mac);
		return 2;
	}
	
	ret = vap_add_acl_mac(sock, ifname, &st);
	CHK_RET(ret, mode_ioctl); 
}


static mode_ioctl_st s_mode_ioctl_arr[] = {  
	{"get_curr_channel", 			get_str_int, 		(void *)get_curr_channel},
	{"get_curr_power", 				get_str_int, 		(void *)get_curr_power},
	{"set_mode", 					set_str_str_int, 	(void *)set_mode},
	{"get_mode", 					get_str_str, 		(void *)get_mode},
	{"create_vap", 					set_str_str_int, 	(void *)create_vap},
	{"delete_vap", 					set_str, 			(void *)delete_vap},
	{"up_vap", 						set_str, 			(void *)up_vap},
	{"down_vap", 					set_str, 			(void *)down_vap},
	{"vap_set_ssid", 				set_str_str, 		(void *)vap_set_ssid},
	{"vap_set_ssid_hide", 			set_str_int, 		(void *)vap_set_ssid_hide},
	{"down_wifi", 					set_str, 			(void *)down_wifi},
	{"up_wifi", 					set_str, 			(void *)up_wifi},
	{"set_countrycode", 			set_str_int, 		(void *)set_countrycode},
	{"start_scan", 					l_start_scan, 		(void *)start_scan},
	{"stop_scan", 					set_str, 			(void *)stop_scan},
	{"set_encrypt_mode", 			set_str_str_str, 	(void *)set_encrypt_mode},
	{"set_channel", 				set_str_int, 		(void *)set_channel},
	{"set_power", 					set_str_int, 		(void *)set_power},
	{"br_add_if", 					set_str_str, 		(void *)br_add_if},
	{"br_del_if", 					set_str_str, 		(void *)br_del_if},
	{"interface_is_exist", 			get_str_int, 		(void *)interface_is_exist},
	{"get_interface_state", 		get_str_int, 		(void *)get_interface_state},
	{"get_radio_users_limit",		get_str_int, 		(void *)get_radio_users_limit},
	{"set_radio_users_limit",		set_str_int, 		(void *)set_radio_users_limit}, 
	{"get_countrycode",				get_str_int, 		(void *)get_countrycode},
	{"set_radio_info",				l_set_radio_info, 	(void *)set_radio_info},
	{"set_vap_info",				l_set_vap_info, 	(void *)set_vap_info},
	{"set_assoc_sta_report_cycle",	set_str_int, 		(void *)set_assoc_sta_report_cycle}, 
	{"set_radio_info_report_cycle",	set_str_int, 		(void *)set_radio_info_report_cycle}, 
	{"vap_set_acl_mode",			set_str_int, 		(void *)vap_set_acl_mode}, 
	{"set_radio_scan_sta_cycle",	set_str_int, 		(void *)set_radio_scan_sta_cycle}, 
	{"set_radio_macaddr", 			set_str_str, 		(void *)set_radio_macaddr},
	{"vap_add_acl_mac",				l_vap_add_acl_mac, 	(void *)vap_add_acl_mac},
	
	{NULL, 							NULL, 				NULL}
};

static mode_ioctl_st *find(const char *method) {
	int i = 0;
	for (i = 0; s_mode_ioctl_arr[i].method; i++) {
		if (!strcmp(method, s_mode_ioctl_arr[i].method))
			return &s_mode_ioctl_arr[i];
	}
	return NULL;
}

static int l_apctl(lua_State *L) {
	CHK_STR(L, 1);
	int i;
	const char *method = (const char *)lua_tostring(L, 1);
	mode_ioctl_st *mode_ioctl = find(method);
	if (!mode_ioctl) {
		lua_pushnil(L);
		lua_pushfstring(L, "invalid method %s", method);
		return 2;
	}

 	return mode_ioctl->mode_func(L, mode_ioctl); 
}


static luaL_Reg reg[] = {
	{ "apctl", 		l_apctl},
	{ NULL, 		NULL }
};
 
LUALIB_API int luaopen_apioctl(lua_State *L) {
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		lua_pushnil(L);
		lua_pushfstring(L, "socket fail %s", strerror(errno));
		return 2;
	} 
	luaL_register(L, "apioctl", reg);
	return 1;
}
