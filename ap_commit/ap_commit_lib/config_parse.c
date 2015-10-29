#include "wireless.h"
#include <string.h>

extern struct wl_mgmt_st g_wl_mgmt;
extern int32_t clear_all_radio_config();
extern char* create_vap_name(RF_TYPE type, char *vap_name, uint8_t len, uint16_t wlan_id);

/**
* @brief		reboot
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int32_t create_launch_flag()
{
	int32_t fd = -1;
	/*标记存在*/
	if (access(COMMIT_LIB_LAUNCH_FLAG, F_OK) == 0)
	{
		return WL_SUCCESS;
	}
	fd = open(COMMIT_LIB_LAUNCH_FLAG, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	if (fd == -1)
	{
		return WL_FAILED;
	}
	return WL_SUCCESS;
}


/**
* @brief		检测launch flag是否存在
* @param
* @return	1 existence, 0 nonexistence
* @remark null
* @see
* @author tgb
**/
int32_t check_launch_flag()
{
	if (access(COMMIT_LIB_LAUNCH_FLAG, F_OK) == 0)
	{
		return 1;
	}
	LOG_DEBUG("err:%s, %s.", COMMIT_LIB_LAUNCH_FLAG, strerror(errno));
	return 0;
}


static int32_t basic_config_parse(const struct ap_cfg_info_st *info)
{	
	struct ap_basic_cfg_st *new_basic_cfg = (struct ap_basic_cfg_st*)((uint8_t*)info + CONF_HEADER_SIZE);

	g_wl_mgmt.basic_info.basic_cfg.country_code = new_basic_cfg->country_code;
	g_wl_mgmt.basic_info.basic_cfg.debug_enable = new_basic_cfg->debug_enable;
	return WL_SUCCESS;
}


/**
* @brief		获取radio节点
* @param	band_type(2g或者5G)
* @return
* @remark null
* @see
* @author tgb
**/
struct wl_radio_st *get_wl_radio(uint8_t rf_type)
{	
	char *wifi_name = NULL;
	struct wl_radio_st *radio_node = NULL;
	
	if (rf_type != RF_2G && rf_type != RF_5G)
	{
		LOG_WARN("unknown rf_type[%d].", rf_type);
		return NULL;
	}

	int i = 0;
	for (i = 0; i < AP_RF_MAX_SIZE; i++)
	{
		if (rf_type != g_wl_mgmt.radio[i].rf_type)
			continue;

		wifi_name = (char*)&g_wl_mgmt.radio[i].wifi_name;
		if (g_wl_mgmt.radio[i].assist.is_support)
		{
			radio_node = &g_wl_mgmt.radio[i];
			break;
		}
	}
	assert(wifi_name);
	
	if (!radio_node)
	{
		LOG_INFO("%s not spport!", wifi_name);
	}
	return radio_node;
}


/**
* @brief		返回列表中第一个可用的vap节点
* @param	band_type(2g或者5G)
**/
static struct wl_vap_st *get_wlan_config_buffer(uint8_t rf_type)
{
	struct wl_radio_st *radio = get_wl_radio(rf_type);
	if (radio == NULL)
		return NULL;	

	int i = 0;
	for (i = 0; i < RF_VAP_MAX_SIZE; i++)
	{
		if (radio->vaps[i].is_used == 0)
		{
			radio->vaps[i].is_used = 1;
			return (&radio->vaps[i]);
		}
	}
	
	return NULL;
}



static int32_t radio_config_update(struct wl_radio_st *radio, struct radio_cfg_st *new_radio_cfg)
{
	assert(radio);
	assert(new_radio_cfg);
	struct radio_cfg_st *radio_cfg = &radio->radio_cfg;
	
	radio_cfg->rf_enable = new_radio_cfg->rf_enable;
	safe_strncpy(radio_cfg->rf_mode , new_radio_cfg->rf_mode, RADIO_MODE_MAX_SIZE);
	radio_cfg->channel_width = new_radio_cfg->channel_width;
	radio_cfg->channel_id = new_radio_cfg->channel_id;
	radio_cfg->rf_power = new_radio_cfg->rf_power;
	radio_cfg->rf_users_limit = new_radio_cfg->rf_users_limit;
	radio_cfg->vap_bridge = new_radio_cfg->vap_bridge;
	radio_cfg->mult_inspeed = new_radio_cfg->mult_inspeed;
	radio_cfg->sta_rate_limit = new_radio_cfg->sta_rate_limit;
	radio_cfg->mult_optim_enable = new_radio_cfg->mult_optim_enable;
	radio_cfg->fairtime_enable = new_radio_cfg->fairtime_enable;
	radio->assist.is_configured = 1;
	
	return WL_SUCCESS;
}


static int32_t radio_config_parse(const struct ap_cfg_info_st *info)
{ 
    struct wl_radio_st *radio = get_wl_radio(info->rf_type);
    if (radio == NULL)
        return WL_SUCCESS;
    struct radio_cfg_st *radio_cfg = (struct radio_cfg_st *)((uint8_t*)info + CONF_HEADER_SIZE); 
    
    return radio_config_update(radio, radio_cfg);
}


static int32_t update_wlan_config(struct wl_vap_st *vap, struct wlan_cfg_st *wlan_cfg, uint8_t rf_type)
{
	struct wlan_cfg_st *wlan = &vap->wlan;
	wlan->wlan_id = wlan_cfg->wlan_id;
	wlan->hide = wlan_cfg->hide;
	safe_strncpy(wlan->ssid, wlan_cfg->ssid, SSID_MAX_SIZE);
	wlan->wlan_enable = wlan_cfg->wlan_enable;
	
	if (create_vap_name(rf_type, vap->vap_name, IF_NAME_LEN, wlan_cfg->wlan_id) == NULL)
	{
		LOG_WARN("create vap name failed.");
		return WL_FAILED;
	}
	
	return WL_SUCCESS;
}


static int32_t wlan_config_parse(const struct ap_cfg_info_st *info)
{
	struct wl_vap_st *vap = NULL;
	struct wlan_cfg_st *wlan_cfg = (struct wlan_cfg_st*)((uint8_t*)info + CONF_HEADER_SIZE);
	
	int i = 0;
	for (i = RF_2G; i <= RF_5G; i++)
	{
		if (!(i & wlan_cfg->rf_type))
			continue;

		vap = get_wlan_config_buffer(i);
		if (vap == NULL)
			continue;

		if (update_wlan_config(vap, wlan_cfg, i) == WL_FAILED)
			return WL_FAILED;
	}
	
	return WL_SUCCESS;
}


/**
* @brief		配置解析前准备操作
**/
static void config_parse_prepare()
{
	clear_all_radio_config();
}

static int32_t config_parse(const struct ap_cfg_info_st *info)
{  
  if (info == NULL ||  info->magic != WL_MGMT_MAGIC)
  {
  	LOG_WARN("MAGIC ERR. info=[0x%p]", info);
	return WL_FAILED;
  }

  int ret = WL_SUCCESS;  
  switch(info->conf_type)
  {
    case CONF_BASIC:
		ret = basic_config_parse(info);
		break;

    case CONF_RADIO:
		ret = radio_config_parse(info);
      break;

    case CONF_WLAN:
		ret = wlan_config_parse(info);
      break;
	  
    default:
      ret = WL_FAILED;
      break;
  }
  
  LOG_INFO("*** Parse config(type=%d). result=%d.", info->conf_type, ret);
  return ret;
}


static int32_t config_commit()
{
	struct wl_radio_st *radio = NULL;

	if (proc_country_update() == WL_FAILED)	
		return WL_FAILED;
	
	if (proc_radio_mode_update() == WL_FAILED)
		return WL_FAILED;
	
	/*全量提交更新*/
	int i = 0;
	for(i = RF_2G; i <= RF_5G; i++)
	{
		radio = get_wl_radio(i);
		if (radio == NULL)
		{
			continue;
		}
		if (full_commit_per_radio(radio) == WL_FAILED)
		{
			return WL_FAILED;
		}
	}
	if (proc_debug_sw_update() == WL_FAILED)
	    return WL_FAILED;

	return WL_SUCCESS;
}


/**
* @brief		配置下发入口 (main)
**/
int32_t config_deliver(const struct ap_cfg_info_st  *cfg_info)
{
	int32_t ret = WL_SUCCESS;
	const struct ap_cfg_info_st *info = cfg_info;
	if (info == NULL)
	{
		LOG_DEBUG("Input a invalid config.");
		return WL_FAILED;
	}
	
	config_parse_prepare();	
	
	while(info)
	{
		ret = config_parse(info);
		if (ret == WL_FAILED)
		{
			LOG_WARN("Parsing config failed.");
			return ret;
		}
		
		if (info->more)
		{	
			info = (struct ap_cfg_info_st *)((uint8_t*)info + CONF_HEADER_SIZE + info->len);
		}
		else
		{
			info = NULL;
		}
	}
	
	ret = config_commit();
	create_launch_flag();	
	
	LOG_INFO("*** Committing over.  result=%s .", (ret==0 ? "OK":"ERROR"));
	return ret;
}

