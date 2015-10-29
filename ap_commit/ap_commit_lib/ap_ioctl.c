#include "ap_ioctl.h"

/**
* @brief    	设置参数到驱动
* @param
* @return
* @remark null
* @see
* @author 
**/
int atheros_set80211priv(int sock, const char *if_name, int option, void *data, size_t len)
{
    if (sock < 0 || NULL == if_name || NULL == data)
	{
		LOG_WARN("Invalid argument.");
		return WL_FAILED;
	}
    
	struct iwreq iwr;
	memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, if_name, IFNAMSIZ);
    
    if (len < IFNAMSIZ)
    {
        memcpy(iwr.u.name, data, len);
    }
    else
    {
        iwr.u.data.pointer = data;
        iwr.u.data.length = len;
    }

    int err = ioctl(sock, option, &iwr);
	if (err < 0)
	{
		LOG_WARN("atheros_set80211priv %s option[%d] datalen:%d set fail:%s.", \
            if_name, option, len, strerror(errno));
	}

	return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/**
* @brief    	设置参数到驱动
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_param(int sock, const char *if_name, int option, int flag)
{
    if (sock < 0 || NULL == if_name)
	{
		LOG_WARN("Invalid argument.");
		return WL_FAILED;
	}
    
	int param[2];
	param[0] = option;
	param[1] = flag;

	return atheros_set80211priv(sock, if_name, IEEE80211_IOCTL_SETPARAM, \
		(void *)&param, sizeof(param));
}


/**
* @brief    	设置信道到驱动
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_channel_id(int sock, const char *if_name, int channel_id)
{
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }
    
	struct iwreq iwr = { { { 0 } } };
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
	iwr.u.freq.m = channel_id;
	iwr.u.freq.e = 0;
	iwr.u.freq.flags = IW_FREQ_FIXED;

    int err = ioctl(sock, SIOCSIWFREQ, &iwr);
	if (err < 0)
	{
		LOG_WARN("set %s channel %u fail:%s.", \
            if_name, channel_id, strerror(errno));
	}

	return err < 0 ? WL_FAILED : WL_SUCCESS;
}


int atheros_set_power(int sock, const char *if_name, int power_val)
{
    if (sock < 0 || NULL==if_name)
    {
        LOG_WARN("Invalid argument.");
        return -1;
    }

	struct iwreq iwr = { { { 0 } } };
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
	iwr.u.txpower.fixed = 1;
	iwr.u.txpower.disabled = 0;
	iwr.u.txpower.flags = IW_TXPOW_DBM;
	iwr.u.txpower.value = power_val;

    int err = ioctl(sock, SIOCSIWTXPOW, &iwr);
	if (err < 0)
	{
		LOG_WARN("set %s Tx txpower %d failed:%s.", if_name, power_val, strerror(errno));
	}

	return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/**
* @brief    	设置信道带宽到驱动
* @param
* @return
* @remark null
* @see
* @author tgb
**/
static void atheros_qmosar_set_cw(int sock, const char *ifname, char *mode)
{
    unsigned int  cwmenable = 0;
    unsigned int  disablecoext = 0;    
    
    if (!strcmp(mode, "11NGHT40") || !strcmp(mode, "11NGHT40PLUS") || 
        !strcmp(mode, "11NGHT40MINUS") || !strcmp(mode, "11NAHT40") || 
        !strcmp(mode, "11NAHT40MINUS") || !strcmp(mode, "11NAHT40MINUS"))
    {
        cwmenable = 1;
        disablecoext = 1;
    }
    
    int err = atheros_set_param(sock, ifname, IEEE80211_PARAM_CWM_ENABLE, cwmenable);    
    int err2 = atheros_set_param(sock, ifname, IEEE80211_PARAM_COEXT_DISABLE, disablecoext);     
    LOG_WARN("result = %d,%d. para[ifname=%s, mode=%s].", err, err2, ifname, mode);
}


/**
* @brief    	设置无线协议
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_mode(int sock, const char *if_name, const char *mode_str)
{	
	assert(if_name);
	assert(mode_str);
	
	#define MODE_LEN 50
    if (sock < 0 || NULL == if_name || NULL == mode_str)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }

	struct iwreq iwr = { { {0} } };
	char mode[MODE_LEN + 1] = {0};
	int len = strlen(mode_str);
	if (len > MODE_LEN)
	{
		len = MODE_LEN;
	}	
	strncpy(mode, mode_str, len);
	strupper(mode);	//全部转为大写
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
	iwr.u.data.pointer = (void *)mode;
	iwr.u.data.length = strlen(mode) + 1;    /*+1不然容易出现Invalid argument */

    atheros_set_channel_id(sock, if_name, 0);
    
    int err = ioctl(sock, IEEE80211_IOCTL_SETMODE, &iwr);
	if (err < 0)
	{
		LOG_WARN("%s set %s rf mode %s fail:%s.", __func__, if_name, mode, strerror(errno));
		goto ERR;
	}
    atheros_qmosar_set_cw(sock, if_name, mode);
ERR:
	return err < 0 ? errno : 0;
}


/**
* @brief    	获取无线协议
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_get_mode(int sock, const char *if_name, char *mode, int len)
{
	assert(if_name);
	assert(mode);
	#define GET_RADIO_MODE "iwpriv %s get_mode | grep get_mode| "\
							"awk '{print $2}' | cut -b 10- 2>&1"
	int32_t ret = 0;
	char cmd_str[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN] = {0};
	snprintf(cmd_str, POPEN_CMD_LEN, GET_RADIO_MODE, if_name);
	if (custom_popen(cmd_str, POPEN_CMD_LEN, 
					 result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	if (strlen(result))
	{
		result[strlen(result) - 1] = '\0';
	}
	safe_strncpy(mode, result, len);
	ret = WL_SUCCESS;
OUT:
	#undef GET_RADIO_MODE
	return WL_SUCCESS;
}



int atheros_create_vap(int sock, const char *if_name, const char *vap_name)
{
    if (sock < 0 || NULL == if_name || NULL == vap_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }

    struct ifreq ifr;
    struct ieee80211_clone_params cp;
    
    memset(&ifr, 0, sizeof(ifr));
    memset(&cp, 0, sizeof(cp));

    strncpy(cp.icp_name, vap_name, IFNAMSIZ);   /*ath2xxx,ath5xxx*/
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);	/*wifix*/
    cp.icp_opmode = IEEE80211_M_HOSTAP;
    cp.icp_flags = IEEE80211_CLONE_BSSID;
    
    ifr.ifr_data  = (void *) &cp;

    int err = ioctl(sock, SIOC80211IFCREATE, &ifr);
    if (err < 0)
    {
        LOG_WARN("%s create %s failed:%s.", if_name, vap_name, strerror(errno));
    }
    
    return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/**
* @brief 			生效vap用户数限制配置
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_vap_users_limit(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	assert(radio);
	assert(vap);
	#define VAP_MAX_STA_CMD_FORMAT  "iwpriv %s maxsta %d"

	uint8_t read_cnt = 0;
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};

	snprintf(cmd, POPEN_CMD_LEN, VAP_MAX_STA_CMD_FORMAT, vap->vap_name, radio->radio_cfg.rf_users_limit);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef VAP_MAX_STA_CMD_FORMAT
    return ret;
}


/**
* @brief 			生效ap隔离，也即vap是否开启桥接
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_vap_bridge(struct wl_vap_st *vap, uint8_t bridge)
{
	assert(vap);
	#define VAP_BRIDGE_FORMAT  "iwpriv %s ap_bridge %d"

	uint8_t read_cnt = 0;
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};

	snprintf(cmd, POPEN_CMD_LEN, VAP_BRIDGE_FORMAT, vap->vap_name,  bridge);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef VAP_BRIDGE_FORMAT
    return ret;
}


/**
* @brief 			生效ap隔离，也即vap是否开启桥接
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_vap_fairtime(struct wl_vap_st *vap, uint8_t fairtime)
{
	assert(vap);
	#define VAP_FAIRTIME_FORMAT  "iwpriv %s fairtime %d"

	uint8_t read_cnt = 0;
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};

	snprintf(cmd, POPEN_CMD_LEN, VAP_FAIRTIME_FORMAT, vap->vap_name, fairtime);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef VAP_FAIRTIME_FORMAT
    return ret;
}


/**
* @brief 			多播优化
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_multi_optim(struct wl_radio_st *radio, uint8_t mult_optim)
{
	assert(radio);
	#define MULTI_OPTIM_FORMAT  "iwpriv %s drop_b_probe %d"

	uint8_t read_cnt = 0;
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};

	snprintf(cmd, POPEN_CMD_LEN, MULTI_OPTIM_FORMAT, radio->wifi_name,  mult_optim);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef MULTI_OPTIM_FORMAT
    return ret;
}


extern int construct_rate(uint8_t rf_type, char *rf_mode, uint8_t rate, uint32_t *com_rate);

int atheros_set_multi_inspeed(struct wl_radio_st *radio, uint8_t mult_speed)
{
	assert(radio);
	#define MULTI_INSPEED_FORMAT  "iwpriv %s min_bcast_rate %d"
	int32_t ret = WL_SUCCESS;
	uint32_t com_rate = 0;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	ret = construct_rate(radio->rf_type, radio->radio_cfg.rf_mode, mult_speed, &com_rate);
	if (ret == WL_FAILED)
	{
		goto OUT;
	}
	snprintf(cmd, POPEN_CMD_LEN, MULTI_INSPEED_FORMAT, radio->wifi_name, com_rate);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef MULTI_INSPEED_FORMAT
    return ret;
}


int atheros_set_sta_rate_limit(struct wl_radio_st *radio, uint8_t rate)
{
	assert(radio);
	#define RATE_LIMIT_FORMAT  "iwpriv %s sta_rate_limit %d"

	int32_t ret = WL_SUCCESS;
	uint32_t com_rate = 0;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	ret = construct_rate(radio->rf_type, radio->radio_cfg.rf_mode, rate, &com_rate);
	if (ret == WL_FAILED)
	{
		goto OUT;
	}
	snprintf(cmd, POPEN_CMD_LEN, RATE_LIMIT_FORMAT, radio->wifi_name,  com_rate);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef RATE_LIMIT_FORMAT
    return ret;
}


/**
* @brief    	down vap
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_down_vap(int sock, char *if_name)
{
	#define DOWN_VAP_CMD_FORMAT  "ifconfig %s down"
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument\n");
        return -1;
    }
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	
	snprintf(cmd, POPEN_CMD_LEN, DOWN_VAP_CMD_FORMAT, if_name);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
OUT:
	#undef DOWN_VAP_CMD_FORMAT
    return ret;	
}


/**
* @brief    	up vap
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_up_vap(int sock, const char *if_name)
{
	#define UP_VAP_CMD_FORMAT  "ifconfig %s up"
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument\n");
        return -1;
    }
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	
	snprintf(cmd, POPEN_CMD_LEN, UP_VAP_CMD_FORMAT, if_name);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
OUT:
	#undef UP_VAP_CMD_FORMAT
    return ret;	
}


/**
* @brief    	设置vap的SSID名称
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_vap_set_ssid(int sock, const char *if_name, const char *essid)
{
    if (sock < 0 || NULL==if_name || NULL==essid)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }
    
    struct iwreq iwr = { { { 0 } } };
    int essid_len = strlen(essid);
    
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
    iwr.u.essid.flags = 1;
    iwr.u.essid.length = essid_len + 1;
    iwr.u.essid.pointer = (char *)essid;

    int err = ioctl(sock, SIOCSIWESSID, &iwr);
    if (err < 0)
    {
        LOG_WARN("%s Set SSID %s failed:%s.", if_name, essid, strerror(errno));
    }

    return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/**
* @brief    	设置SSID隐藏
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_vap_set_ssid_hide(int sock, const char *if_name, int ssid_hide)
{
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }

    return atheros_set_param(sock, if_name, IEEE80211_PARAM_HIDESSID, ssid_hide);    
}


/**
* @brief    	down wifi
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_down_wifi(int sock, const char *if_name)
{
	#define DOWN_WIFI_CMD_FORMAT  "ifconfig %s down"
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument\n");
        return -1;
    }
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	
	snprintf(cmd, POPEN_CMD_LEN, DOWN_WIFI_CMD_FORMAT, if_name);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
OUT:
	#undef DOWN_WIFI_CMD_FORMAT
    return ret;	
}


/**
* @brief    	up wifi
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_up_wifi(int sock, const char *if_name)
{
	#define UP_WIFI_CMD_FORMAT  "ifconfig %s up"
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument\n");
        return -1;
    }
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	
	snprintf(cmd, POPEN_CMD_LEN, UP_WIFI_CMD_FORMAT, if_name);
	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
OUT:
	#undef UP_WIFI_CMD_FORMAT
    return ret;	
}


/**
* @brief    	设置国家码到驱动
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_set_countrycode(int sock, const char *if_name, int country_code)
{
    if (sock < 0 || NULL == if_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }

    return atheros_set_param(sock, if_name, \
        SPECIAL_PARAM_SHIFT|SPECIAL_PARAM_COUNTRY_ID, country_code);
}


/**
* @brief    	从驱动获取国家码
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_get_countrycode(int sock, const char *if_name, int *country_code)
{
	assert(if_name);
	int32_t ret = 0;
	#define GET_COUNTRY_CMD_FORMAT "iwpriv %s getCountryID | grep "\
								"getCountryID | awk '{print $2}'| cut -c 14- 2>&1"
	char cmd_str[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN] = {0};
	snprintf(cmd_str, POPEN_CMD_LEN, GET_COUNTRY_CMD_FORMAT, if_name);
	if (custom_popen(cmd_str, POPEN_CMD_LEN, 
					 result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	*country_code = atoi(result);
	ret = WL_SUCCESS;
OUT:
	#undef GET_COUNTRY_CMD_FORMAT
	return ret;
}


/**
* @brief    添加接口到桥
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_br_add_if(int sock, const char *br_name, const char *if_name)
{
    if (sock < 0 || NULL == br_name || NULL == if_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }
    
    struct ifreq ifr;
    int err = 0;
    unsigned int if_index = if_nametoindex(if_name);

    if (0 == if_index)
    {
        LOG_WARN("No such device:%s.", if_name);
        return -1;
    }
    
    strncpy(ifr.ifr_name, br_name, IFNAMSIZ);
    
#ifdef SIOCBRADDIF
    ifr.ifr_ifindex = if_index;
    err = ioctl(sock, SIOCBRADDIF, &ifr);
    if (err < 0)
#endif
    {
        unsigned long args[4] = { BRCTL_ADD_IF, if_index, 0, 0 };
        ifr.ifr_data = (char *) args;
        err = ioctl(sock, SIOCDEVPRIVATE, &ifr);
    }

    if (err < 0 && EBUSY == errno)//接口存在,不需要添加
    {
        LOG_WARN("device:%s is already a member of a bridg:%s.", \
            if_name, br_name);
        err = 0; //不报错
    }

   return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/**
* @brief    删除桥中的接口
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int atheros_br_del_if(int sock, const char *br_name, const char *if_name)
{
    if (sock < 0 || NULL==br_name || NULL==if_name)
    {
        LOG_WARN("Invalid argument.");
        return WL_FAILED;
    }

    struct ifreq ifr;
    int err;
    int ifindex = if_nametoindex(if_name);
    
	if (0 == ifindex)
	{
        LOG_WARN("No such device:%s.", if_name);
        return -1;
	}
    
    strncpy(ifr.ifr_name, br_name, IFNAMSIZ);
    
#ifdef SIOCBRDELIF
    ifr.ifr_ifindex = ifindex;
    err = ioctl(sock, SIOCBRDELIF, &ifr);
    if (err < 0)
#endif
    {
        unsigned long args[4] = { BRCTL_DEL_IF, ifindex, 0, 0 };
        ifr.ifr_data = (char *) args;
        err = ioctl(sock, SIOCDEVPRIVATE, &ifr);
    }

    if (err < 0 && EINVAL == errno)//接口不存在,不需要删
    {
        LOG_WARN("device %s is not a slave of %s.", if_name, br_name);
        err = 0;
    }
    
    return err < 0 ? WL_FAILED : WL_SUCCESS;
}


/*设置信道利用率收集开关*/
int atheros_set_chan_use_sw(int sock, const char *wifi_name, uint8_t debug_sw)
{
	assert(wifi_name);
	int32_t ret = WL_SUCCESS;
	#define CHAN_USE_CMD_FORMAT "iwpriv %s chan_use_en %d"
	char cmd_str[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN] = {0};
	snprintf(cmd_str, POPEN_CMD_LEN, CHAN_USE_CMD_FORMAT, wifi_name, debug_sw);
	if (custom_popen(cmd_str, POPEN_CMD_LEN, 
					 result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	ret = WL_SUCCESS;
OUT:
	#undef CHAN_USE_CMD_FORMAT
	return ret;
}