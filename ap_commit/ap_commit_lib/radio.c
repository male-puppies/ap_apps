#include "wireless.h"

typedef int32_t (*rf_cfg_check)(struct wl_radio_st *radio, const void *new_cfg);
typedef int32_t (*rf_cfg_set)(struct wl_radio_st *radio, const void *new_cfg);

/*射频参数设置结构*/
struct rf_cfg_item_st
{
    uint8_t         index;      /*配置索引*/
    rf_cfg_check    check_func; /*参数有消息校验函数*/
    rf_cfg_set      set_func;   /*参数设置函数*/
    uint8_t         down_radio; /*是否需要down radio*/
    uint8_t         down_vap;   /*是否需要down vap*/
	uint8_t			full_commit;/*是否全量下发*/
};


/**
* @brief		 设备检测, 检测是否存在对应wifi
* @param 	radio_name    radio对应的wifi name 
* @return	0    radio存在；-1   radio不存在
* @remark null
* @see
* @author tgb
**/
int32_t radio_probe(struct wl_radio_st *radio)
{
	#define EXT_CMD_LEN 64
	#define WIFI_SUPPORT_CMD_FORMAT "ifconfig -a | grep %s | wc -l"
	FILE *cmd_stream = NULL;
	uint8_t read_cnt = 0;
    int32_t ret = WL_FAILED;
	char cmd[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN]= {0};
	snprintf(cmd, POPEN_CMD_LEN, WIFI_SUPPORT_CMD_FORMAT, radio->wifi_name);
	if (custom_popen(cmd , POPEN_CMD_LEN, result, POPEN_RESULT_LEN) == WL_FAILED)
	{
		ret = WL_FAILED;
		goto OUT;
	}
	if (atoi(result))
	{
		ret = WL_SUCCESS;
	}
	else
	{
		ret = WL_FAILED;
	}
OUT:
	#undef WIFI_SUPPORT_CMD_FORMAT
    return ret;
}


/**
* @brief		设置radio使能位
* @param 	radio
* @param 	new_cfg 对应的radio配置
* @return	
* @remark null
* @see
* @author tgb
**/
int32_t commit_rf_enable(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	int32_t ret = WL_FAILED;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	if (radio_cfg->rf_enable)
	{
		ret = atheros_up_wifi(radio->ioctl_sock, radio->wifi_name);
		LOG_DEBUG("Up radio:wifi%d, ret:%d.", 
					(radio_cfg->rf_type == RF_2G ? 0 : 1), ret);
	}
	else
	{
		ret = atheros_down_wifi(radio->ioctl_sock, radio->wifi_name);
		LOG_DEBUG("Down radio:wifi%d, ret:%d.", 
					(radio_cfg->rf_type == RF_2G ? 0 : 1), ret);
	}
	return ret;
}


/**
* @brief   	由基本模式及带宽生成模式
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
static int32_t s_11n_param_from_cw(uint8_t rf_type, uint8_t channel_width, char *mode)
{
    assert(mode != NULL);
    int set_11ac = 0;

    if ((rf_type != RF_2G) && (rf_type != RF_5G))
    {
        LOG_DEBUG("Wrong rf_type %u", rf_type);
        return -1;
    }
	
    switch(channel_width)
    {
        case BANDWIDTH_20:
            if (RF_2G == rf_type)
            {
                safe_strncpy(mode, "11NGHT20", RADIO_MODE_MAX_SIZE);
            }
            else
            {   
				safe_strncpy(mode, "11NAHT20", RADIO_MODE_MAX_SIZE);
            }
            break;
        case BANDWIDTH_40_PLUS:
            if (RF_2G == rf_type)
            {
                safe_strncpy(mode, "11NGHT40PLUS", RADIO_MODE_MAX_SIZE);
            }
            else
            {
				safe_strncpy(mode, "11NAHT40PLUS", RADIO_MODE_MAX_SIZE);
            }
            break;
        case BANDWIDTH_40_MINUS:
            if (RF_2G == rf_type)
            {
                safe_strncpy(mode, "11NGHT40MINUS", RADIO_MODE_MAX_SIZE);
            }
            else
            {          
				safe_strncpy(mode, "11NAHT40MINUS", RADIO_MODE_MAX_SIZE);
            }
            break;
        case BANDWIDTH_AUTO:
            if (RF_2G == rf_type)
            {
                safe_strncpy(mode, "11NGHT40", RADIO_MODE_MAX_SIZE);
            }
            else
            {       
				safe_strncpy(mode, "11NAHT40", RADIO_MODE_MAX_SIZE);
            }
            break;
        default:
            LOG_WARN("Wrong channel_width %u", channel_width);
            return -1;
            
    }
    return 0;
}


/**
* @brief		构造模式
* @param 	radio
* @param 	
* @return	
* @remark null
* @see
* @author tgb
**/
int32_t construct_mode(uint8_t rf_type, char *rf_mode, uint8_t channel_width, 
							char *mode, uint8_t len)
{
	assert(rf_mode);
	int32_t ret = WL_SUCCESS;
	if (rf_type == RF_2G)
	{
		if (strcmp(rf_mode, "bg") == 0 
		    || strcmp(rf_mode, "b") == 0 
			|| strcmp(rf_mode, "g") == 0)
		{
			safe_strncpy(mode, "11G", RADIO_MODE_MAX_SIZE);
		}		
		else if (strcmp(rf_mode, "bgn") == 0 || strcmp(rf_mode, "n") == 0)
		{
			ret = s_11n_param_from_cw(rf_type, channel_width, mode);
		}
		else
		{
			ret = WL_FAILED;
		}
	}
	else
	{
		if (strcmp(rf_mode, "a") == 0 || strcmp(rf_mode, "n") == 0 
			|| strcmp(rf_mode, "an") == 0)
		{
			ret = s_11n_param_from_cw(rf_type, channel_width, mode);
		}
		else
		{
			ret = WL_FAILED;
		}
	}
	
	LOG_DEBUG("rf_type:%d, rf_mode:%s, mode:%s.", rf_type, rf_mode, mode);
	return ret;
}


int construct_rate(uint8_t rf_type, char *rf_mode, uint8_t rate, uint32_t *com_rate)
{
	uint32_t rf_mode_type = 0;
	if (rf_type == RF_2G)
	{
		if (strcmp(rf_mode, "bg") == 0 
		    || strcmp(rf_mode, "b") == 0 
			|| strcmp(rf_mode, "g") == 0)
		{
			rf_mode_type = 3;
		}		
		else if (strcmp(rf_mode, "bgn") == 0 || strcmp(rf_mode, "n") == 0)
		{
			rf_mode_type = 5;
		}
		else
		{
			return WL_FAILED;
		}
	}
	else
	{
		if (strcmp(rf_mode, "a") == 0 || strcmp(rf_mode, "n") == 0 
			|| strcmp(rf_mode, "an") == 0)
		{
			rf_mode_type = 7;
		}
		else
		{
			return WL_FAILED;
		}
	}
	rf_mode_type *= 256;
	if (rf_type == RF_2G)
	{
		rf_mode_type += 4096;
	}
	else
	{
		rf_mode_type += 8192;
	}
	rf_mode_type += rate;
	*com_rate = rf_mode_type;
	return WL_SUCCESS;
}


/**
* @brief		设置radio模式
* @param 	radio
* @param 	new_cfg 对应的radio配置
* @return	
* @remark null
* @see
* @author tgb
**/
int32_t commit_rf_mode(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	char mode[RADIO_MODE_MAX_SIZE];
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap = NULL;
	struct radio_cfg_st *radio_cfg = NULL;
	
	radio_cfg = (struct radio_cfg_st*)new_cfg;
	LOG_DEBUG("radio:%s, mode:%s", radio->wifi_name, radio_cfg->rf_mode);
	if (construct_mode(radio->rf_type, radio_cfg->rf_mode, 
				radio_cfg->channel_width, mode, RADIO_MODE_MAX_SIZE) == WL_FAILED)
	{
		return WL_FAILED;
	}
	
	LOG_DEBUG("radio:%s, mode&cw:%s", radio->wifi_name, mode);
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_mode(radio->ioctl_sock, vap->vap_name, mode);
		if (ret == WL_FAILED)
		{
			LOG_DEBUG("set mode %s of %s failed." , mode, vap->vap_name);
			break;
		}
		else
		{
			LOG_DEBUG("set mode %s of %s success.", mode, vap->vap_name);
		}
	}
	LOG_INFO("radio:%s, mode:%s, ret:%d", radio->wifi_name, radio_cfg->rf_mode, ret);
	return ret;
}


/**
* @brief   设置信道
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t commit_channel_id(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);	
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap =  NULL;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_channel_id(radio->ioctl_sock, vap->vap_name, radio_cfg->channel_id);
		if (ret == WL_FAILED)
		{
			LOG_DEBUG("set channel %d of %s failed." , radio_cfg->channel_id, vap->vap_name);
			continue;
		}
		else
		{
			LOG_DEBUG("set channel %d of %s success.", radio_cfg->channel_id, vap->vap_name);
			//break;
		}
	}
	LOG_INFO("radio:%s, channel_id: %d, ret:%d.", radio->wifi_name, radio_cfg->channel_id, ret);
	return ret;
}


/**
* @brief   	设置功率
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t commit_rf_power(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap =  NULL;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_power(radio->ioctl_sock, vap->vap_name, radio_cfg->rf_power);
		if (ret == WL_FAILED)
		{
			LOG_DEBUG("set power %d of %s failed." , radio_cfg->rf_power, vap->vap_name);
			continue;
		}
		else
		{
			LOG_DEBUG("set power %d of %s success.", radio_cfg->rf_power, vap->vap_name);
			break;
		}
	}
	LOG_INFO("radio:%s, power:%d, ret:%d", radio->wifi_name, radio_cfg->rf_power, ret);
	return ret;
}


/**
* @brief   设置支持的最大用户数
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t commit_vap_users_limit(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap = NULL;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_vap_users_limit(radio, vap);
		if (ret == WL_FAILED)
		{
			break;
		}
	}
	return ret;
}


/**
* @brief   设置virtual_ap隔离
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t commit_vap_bridge(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap = NULL;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_vap_bridge(vap, radio_cfg->vap_bridge);
		if (ret == WL_FAILED)
		{
			break;
		}
	}
	return ret;
}


int32_t commit_vap_fairtime(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_vap_st *vap = NULL;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	for (vap_idx = 0; vap_idx < RF_VAP_MAX_SIZE; vap_idx++)
	{
		vap = &radio->vaps[vap_idx];
		if (vap->is_used == 0)
		{
			continue;
		}
		ret = atheros_set_vap_fairtime(vap, radio_cfg->fairtime_enable);
		if (ret == WL_FAILED)
		{
			break;
		}
	}
	return ret;
}


/**
* @brief   	设置多播优化
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t commit_rf_multi_optim(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	ret = atheros_set_multi_optim(radio, radio_cfg->mult_optim_enable);
	LOG_INFO("radio:%s, multi_optim:%d, ret:%d", 
			radio->wifi_name, radio_cfg->mult_optim_enable, ret);
	return ret;
}


int32_t commit_rf_multi_inspeed(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	ret = atheros_set_multi_inspeed(radio, radio_cfg->mult_inspeed);
	LOG_INFO("radio:%s, multi_inspeed:%d, ret:%d", 
				radio->wifi_name, radio_cfg->mult_inspeed, ret);
	return ret;
}


int32_t commit_rf_sta_rate_limit(struct wl_radio_st *radio, const void *new_cfg)
{
	assert(radio);
	assert(new_cfg);
	uint8_t vap_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct radio_cfg_st *radio_cfg = (struct radio_cfg_st*)new_cfg;
	LOG_INFO("start...");
	ret = atheros_set_sta_rate_limit(radio, radio_cfg->sta_rate_limit);
	LOG_INFO("radio:%s, sta_rate_limit:%d, ret:%d.", 
				radio->wifi_name, radio_cfg->sta_rate_limit, ret);
	return ret;
}



/*rf配置项列表*/
struct rf_cfg_item_st g_rf_cfg_blk[] =
{
	{INDEX_RF_ENABLE, NULL,&commit_rf_enable, 0, 0, 1},
    {INDEX_RF_MODE, NULL, &commit_rf_mode, 0, 0, 1},
    {INDEX_CHANNEL_WIDTH, NULL, NULL, 0, 0, 1},
    {INDEX_CHANNEL_ID, NULL, &commit_channel_id, 0, 0, 1},
    {INDEX_RF_POWER, NULL, &commit_rf_power, 0, 0, 0},
    {INDEX_USERS_LIMIT, NULL, &commit_vap_users_limit, 0, 0, 0},
    {INDEX_AMPDU_ENABLE, NULL, NULL, 0, 0, 1},
    {INDEX_AMSUD_ENABLE, NULL, NULL, 0, 0, 1},
    {INDEX_BEACON_INTVAL, NULL, NULL, 0, 0, 1},
    {INDEX_DTIM, NULL, NULL, 0, 0, 1},
    {INDEX_LEAD_CODE, NULL, NULL, 0, 0, 1},
    {INDEX_RETRANS_MAX, NULL, NULL, 0, 0, 0},
    {INDEX_RTS, NULL, NULL, 0, 0, 1},
    {INDEX_SHORTGI, NULL, NULL, 0, 0, 1},
	{INDEX_BRIDGE_ENABLE, NULL, &commit_vap_bridge, 0, 0, 1},
    {INDEX_FAIRTIME_ENABLE, NULL, &commit_vap_fairtime, 0, 0, 1},
	{INDEX_STA_RATE_LIMIT, NULL, &commit_rf_sta_rate_limit, 0, 0, 1},
    {INDEX_MULTI_INSPEED, NULL, &commit_rf_multi_inspeed, 0, 0, 1},
    {INDEX_MULT_OPTIM_ENABLE, NULL, &commit_rf_multi_optim, 0, 0, 1},
    {INDEX_LD_SWITCH, NULL, NULL, 0, 0, 1},
    
	{INDEX_RF_MAX, NULL, NULL, 0, 0, 0},
};


int32_t radio_down(struct wl_radio_st *radio)
{
	int32_t ret = WL_SUCCESS;
	
	ret = atheros_down_wifi(radio->ioctl_sock, radio->wifi_name);
	if (ret == 0)
	{
		ret = WL_SUCCESS;
		LOG_INFO("down %s succes.", radio->wifi_name);
	}
	else
	{
		ret = WL_FAILED;
		LOG_WARN("down %s failed", radio->wifi_name);
	}
	return ret;
}


int32_t radio_up(struct wl_radio_st *radio)
{
	int32_t ret = WL_SUCCESS;
	ret = atheros_up_wifi(radio->ioctl_sock, radio->wifi_name);
	if (ret == 0)
	{
		ret = WL_SUCCESS;
		LOG_INFO("up %s succes.", radio->wifi_name);
	}
	else
	{
		ret = WL_FAILED;
		LOG_WARN("up %s failed", radio->wifi_name);
	}
	return ret;
}


int32_t set_radio_enable(struct wl_radio_st *radio)
{
	assert(radio);
	if (radio->radio_cfg.rf_enable)
	{
		return radio_up(radio);
	}
	else
	{
		return radio_down(radio);
	}
}


/**
* @brief 		生效rf配置
* @param radio
* @return
* @remark null
* @see
* @author tgb
**/
int32_t radio_cfg_commit(struct wl_radio_st *radio)
{
	int32_t ret = WL_SUCCESS;
	uint8_t updated = 0;
	struct rf_cfg_item_st *item = NULL;
	
	LOG_DEBUG("commit config of %s start....", radio->wifi_name);
	int i = 0;
	for (i = 0; i < INDEX_RF_MAX; i ++)
	{
		item = g_rf_cfg_blk + i;
		
		if (i == INDEX_RF_ENABLE)
			continue;
		if (item == NULL || (item && !item->set_func))
			continue;

		ret = item->set_func(radio, &radio->radio_cfg);
		if (ret == WL_FAILED)
		{
			LOG_WARN("radio(wifi%d) commit failed(%s).", radio->rf_type, strerror(errno));
			break;
		}
	}
	
	LOG_DEBUG("commit config of %s end.", radio->wifi_name);
	return ret;
}


/**
* @brief	 清除所有配置标记
* @param 
* @return
* @remark null
* @see
* @author tgb
**/
int32_t clear_all_radio_config()
{
	int i, vap_id;
	for (i=RF_2G; i<=RF_5G; i++)
	{
		struct wl_radio_st *radio = (struct wl_radio_st *)get_wl_radio(i);
		if (radio == NULL)
			continue;
		
		radio->wlan_cnt = 0;
		radio->assist.is_configured = 0;
		for (vap_id=0; vap_id<RF_VAP_MAX_SIZE; vap_id++)
		{
			struct wl_vap_st *vap = &radio->vaps[vap_id];
			SAFE_BEZERO(vap);
		}
	}
	
	LOG_INFO("Clear all vap config.");
	return WL_SUCCESS;
}

