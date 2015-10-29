#include "wireless.h"
#define QMOS_WL_BR_NAME "br0"

/*参数有效性检测*/
typedef int32_t (*wlan_cfg_check_func)(struct wl_radio_st *radio, struct wl_vap_st *vap);
/*参数生效到驱动*/
typedef int32_t (*wlan_cfg_set_func)(struct wl_radio_st *radio, struct wl_vap_st *vap);

struct wlan_cfg_item_st    
{
    int32_t                index;
    wlan_cfg_check_func    check_func; 
    wlan_cfg_set_func      set_func;        /*此处接口需要调整*/
    uint8_t                 down_radio;
    uint8_t                 down_vap;
	uint8_t					full_commit;
};

struct encrypt_pwd_st
{
	char *entryption;
	char *password;
};


/**
* @brief 		生效wlan_enable配置
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
static int32_t  commit_wlan_enable(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	assert(radio);
	assert(vap);
	uint8_t ret = WL_SUCCESS;
	if (vap->wlan.wlan_enable)
	{
		ret = up_vap_proc(radio, vap);
	}
	else
	{
		ret = down_vap_proc(radio, vap);
	}
	LOG_DEBUG("set wlan_enable(%d) of %s; ret:%d", vap->wlan.wlan_enable, vap->vap_name, ret);
	return WL_SUCCESS;
}


/**
* @brief		隐藏ssid
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
static int32_t commit_hide_ssid(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	assert(radio);
	assert(vap);
	int32_t ret = WL_SUCCESS;
	ret = atheros_vap_set_ssid_hide(radio->ioctl_sock, vap->vap_name, vap->wlan.hide);
	LOG_DEBUG("set hide ssid(%d) of wifi%d; ret:%d", vap->wlan.wlan_id,
				(radio->rf_type == RF_2G ? 0 : 1), ret);
	return ret;
}


/**
* @brief 生效wlan配置
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
static int32_t commit_wlan_ssid(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	int32_t ret = WL_SUCCESS;
	ret = atheros_vap_set_ssid(radio->ioctl_sock, vap->vap_name, vap->wlan.ssid);
	
	LOG_DEBUG("set ssid (wlan_id:%d) of wifi%d; ret:%d", vap->wlan.wlan_id,
				(radio->rf_type == RF_2G ? 0 : 1), ret);
				
	return ret;
}

/*wlan配置项列表*/
struct wlan_cfg_item_st g_wlan_cfg_blk[] =
{
    //{INDEX_WLAN_ID, NULL, &commit_wlan_id, 0, 0, 0},
	{INDEX_WLAN_ID, NULL, NULL, 0, 0, 0},
    {INDEX_HIDE_SSID, NULL, &commit_hide_ssid, 0, 0, 1},
    {INDEX_ENCRYPT_AND_PWD, NULL, NULL , 0, 0, 0},
    {INDEX_WLAN_SSID, NULL, &commit_wlan_ssid, 0, 0, 1},
    {INDEX_WLAN_ENABLE, NULL, NULL, 0, 0, 1} 
};


/**
* @brief		构造vap_name
* @举例 type=0,wlan_id=1; 输出vap_name=ath2001
*                type=1,wlan_id=2; 输出vap_name=ath5002
**/
char* create_vap_name(RF_TYPE type, char *vap_name, uint8_t len, uint16_t wlan_id)
{
	assert(vap_name);
	assert(len >= IF_NAME_LEN);	
	char  wlan_id_str[8] = {0};	

	snprintf(wlan_id_str, VAP_NAME_SUFFIX_LEN + 1, "%03d", wlan_id);
	
	if (type == RF_2G)
		snprintf(vap_name, len, VAP_NAME_2G_PREFIX"%s", wlan_id_str);
	else 
        snprintf(vap_name, len, VAP_NAME_5G_PREFIX"%s", wlan_id_str);
	
	LOG_INFO("rftype=%d, VAP_NAME:%s", type, vap_name);
	return vap_name;
}


/**
* @brief 生效wlan配置
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int32_t set_vap(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
    int ret = 0; 
	struct wlan_cfg_item_st *item = NULL;	
	LOG_DEBUG("commit config of %s start...", vap->vap_name);

	int i;
	for (i = 0; i < INDEX_WLAN_MAX; i++)
	{
		item = g_wlan_cfg_blk + i;
		
		if (i == INDEX_WLAN_ENABLE)
			continue;
		if (item == NULL || (item && !item->set_func))
			continue;

		ret = item->set_func(radio, vap);
		if (ret == WL_FAILED)
			break;
	}
	
	LOG_DEBUG("%s: set_vap:%s. i=%d, ret=%d.", radio->wifi_name, vap->vap_name, i,ret);
	return ret;
}


/**
* @brief	销毁指定radio下的所有vap
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int32_t destroy_all_vap(struct wl_radio_st *radio)
{
	assert(radio);
	#define KILL_ALL_VAP_CMD "sh /ugw/scripts/killVAP.sh all"

	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	FILE *cmd_stream = NULL;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};

	safe_strncpy(cmd, KILL_ALL_VAP_CMD, POPEN_CMD_LEN);

	if (custom_popen(cmd, POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
	}
	else
	{
		ret = WL_SUCCESS;
	}
	#undef KILL_ALL_VAP_CMD
	return ret;
}


/**
* @brief		创建指定vap
* @param 	radio 
* @param 	vap
* @return
* @remark 
* @see
* @author tgb
**/
static int32_t vap_create(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	assert(radio);
	assert(vap);
	int32_t ret = WL_FAILED;
	
	ret = atheros_create_vap(radio->ioctl_sock, radio->wifi_name, vap->vap_name);

	LOG_INFO("%s: create %s. ret=%d.", radio->wifi_name, vap->vap_name, ret);
	return ret;
}


/**
* @brief 创建指定radio下的所有vap
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int create_all_vap(struct wl_radio_st *radio)
{
	struct wl_vap_st *vap = NULL;

	int i = 0;
	for (i = 0; i < RF_VAP_MAX_SIZE; i++)
	{
		vap = &radio->vaps[i];
		if (vap->is_used == 0)
			continue;

		if (vap_create(radio, vap) == WL_FAILED)
			return WL_FAILED;
			
		if (atheros_br_add_if(radio->ioctl_sock, QMOS_WL_BR_NAME, vap->vap_name) == WL_FAILED)
		    return WL_FAILED;
	}
	
	return WL_SUCCESS;
}



/**
* @brief 	设置所有的vap的属性
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int32_t set_all_vap(struct wl_radio_st *radio)
{
	struct wl_vap_st *vap = NULL;
	
	int i = 0;
	for (i = 0; i < RF_VAP_MAX_SIZE; i++)
	{
		vap = &radio->vaps[i];
		if (vap->is_used == 0)
			break;

		if (set_vap(radio, vap) == WL_FAILED)
		    return WL_FAILED;
	}
	
	return WL_SUCCESS;
}


/**
* @brief 		up vap
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int32_t up_vap_proc(struct wl_radio_st *radio,struct wl_vap_st *vap)
{
	int32_t ret = WL_SUCCESS;
	ret = atheros_up_vap(radio->ioctl_sock, vap->vap_name);
	if (ret == WL_SUCCESS)
	{
		LOG_INFO("up %s success.", vap->vap_name);
	}
	else
	{
		LOG_WARN("up %s failed.", vap->vap_name);
	}
	return ret;
}


/**
* @brief 		down vap
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int32_t down_vap_proc(struct wl_radio_st *radio, struct wl_vap_st *vap)
{
	assert(radio);
	assert(vap);
	int32_t ret = WL_SUCCESS;
	ret = atheros_down_vap(radio->ioctl_sock, vap->vap_name);
	if (ret == WL_SUCCESS)
	{
		LOG_INFO("down %s success.", vap->vap_name);
	}
	else
	{
		LOG_WARN("down %s failed.", vap->vap_name);
	}
	return ret;
}



/**
* @brief 		生效vap使能位
* @param radio 
* @return
* @remark 
* @see
* @author tgb
**/
int32_t set_all_vap_enable(struct wl_radio_st *radio)
{
	int32_t ret = WL_SUCCESS;
	uint8_t vap_idx = 0;
	struct wl_vap_st *vap = NULL;
	LOG_DEBUG("set all vap enable of %s start....", radio->wifi_name);
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{	/*后续is_used都为0，此处直接break*/
			break;
		}
		if (radio->radio_cfg.rf_enable && vap->wlan.wlan_enable)
		{
			ret = up_vap_proc(radio, vap);
			if (ret == WL_FAILED)
			{	/*up失败，直接break*/
				break;
			}
			else
			{
				continue;
			}
		}
		ret = down_vap_proc(radio, vap);
		if (ret == WL_FAILED)
		{
			break;
		}
	}
	LOG_DEBUG("set all vap enable of %s end.", radio->wifi_name);
	return ret;
}
