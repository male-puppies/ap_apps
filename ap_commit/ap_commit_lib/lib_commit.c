#include "lib_commit.h"
#include "wireless.h"

extern int32_t radio_probe(struct wl_radio_st* radio);	/*外部引用，测试radio是否支持*/

/*全局配置信息*/
static int8_t init_flag = 0;		/*初始化标志，避免多次初始化*/
struct wl_mgmt_st g_wl_mgmt;	/*配置管理结构*/
log_print g_log_print = &printf;


int32_t set_basic_country_code(struct ap_cfg_info_st *obj, const int32_t country_code)
{
	assert(obj);
	struct ap_basic_cfg_st *basic = 
		(struct ap_basic_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_BASIC)
	{
		LOG_WARN("CONF_TYPE(%d) has no element country code.", obj->conf_type);
		return WL_FAILED;
	}
	basic->country_code = country_code;
	return WL_SUCCESS;
}


/**
* @brief		参数设置- 工作模式
* @param 	
* @param 	
* @return	
* @remark  normal, hybrid, monitor三种工作模式
* @see
* @author tgb
**/
int32_t set_basic_work_mode(struct ap_cfg_info_st *obj, const int8_t work_mode)
{
	assert(obj);
	struct ap_basic_cfg_st *basic = 
		(struct ap_basic_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_BASIC)
	{
		LOG_WARN("CONF_TYPE(%d) has no element work mode.", obj->conf_type);
		return WL_FAILED;
	}
	basic->work_mode = work_mode;
	return WL_SUCCESS;
}


/**
* @brief		参数设置-调试开关
* @param 	
* @param 	
* @return	
* @remark  0--关，1--开
* @see
* @author tgb
**/
int32_t set_basic_debug_sw(struct ap_cfg_info_st *obj, const uint8_t debug_enable)
{
	assert(obj);
	struct ap_basic_cfg_st *basic = 
		(struct ap_basic_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_BASIC)
	{
		LOG_WARN("CONF_TYPE(%d) has no element ap desc.", obj->conf_type);
		return WL_FAILED;
	}
	basic->debug_enable = debug_enable ? 1 : 0;
	return WL_SUCCESS;
}


/**
* @brief		参数设置-ap描述
* @param 	
* @param 	
* @return	
* @remark  normal, hybrid, monitor三种工作模式
* @see
* @author tgb
**/
int32_t set_basic_ap_desc(struct ap_cfg_info_st *obj, const char* ap_desc)
{
	assert(obj);
	struct ap_basic_cfg_st *basic = 
		(struct ap_basic_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_BASIC)
	{
		LOG_WARN("CONF_TYPE(%d) has no element ap desc.", obj->conf_type);
		return WL_FAILED;
	}
	safe_strncpy(basic->ap_desc, ap_desc, AP_DESC_MAX_SIZE);
	return WL_SUCCESS;
}



/**
* @brief		参数设置-设备ID
* @param 	
* @param 	
* @return	dev_id: 就是AP的BR0, MAC地址
* @remark  
* @see
* @author tgb
**/
int32_t set_basic_ap_devid(struct ap_cfg_info_st *obj, const uint64_t dev_id)
{
	assert(obj);
	struct ap_basic_cfg_st *basic = 
		(struct ap_basic_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_BASIC)
	{
		LOG_WARN("CONF_TYPE(%d) has no element ap desc.", obj->conf_type);
		return WL_FAILED;
	}
	basic->ap_devid = dev_id;
	return WL_SUCCESS;
}


/**
* @brief		参数设置-radio 射频类型
* @param 	
* @param 	
* @return	
* @remark  
* @see
* @author tgb
**/
int32_t set_radio_rf_type(struct ap_cfg_info_st *obj, const uint8_t rf_type)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (rf_type != RF_2G && rf_type != RF_5G)
	{
		LOG_WARN("Invalid rf_type(%d).", rf_type);
		return WL_FAILED;
	}
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element rf type.", obj->conf_type);
		return WL_FAILED;
	}
	obj->rf_type = rf_type;
	radio->rf_type = rf_type;
	return WL_SUCCESS;
}



int32_t set_radio_rf_enable(struct ap_cfg_info_st *obj, const uint8_t rf_enable)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element rf enable.", obj->conf_type);
		return WL_FAILED;
	}
	radio->rf_enable = rf_enable;
	return WL_SUCCESS;
}



int32_t set_radio_rf_mode(struct ap_cfg_info_st *obj, const char* rf_mode)
{
	assert(obj);
	assert(rf_mode);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element rf mode.", obj->conf_type);
		return WL_FAILED;
	}
	LOG_DEBUG("safe:%p", &safe_strncpy);
	LOG_DEBUG("MODE %s", radio->rf_mode);
	safe_strncpy(radio->rf_mode, rf_mode, RADIO_MODE_MAX_SIZE);
	return WL_SUCCESS;
}



int32_t set_radio_channel_width(struct ap_cfg_info_st *obj, const uint8_t chan_width)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element channel width.", obj->conf_type);
		return WL_FAILED;
	}
	radio->channel_width = chan_width;
	return WL_SUCCESS;
}



int32_t set_radio_channel_id(struct ap_cfg_info_st *obj, const uint8_t chan_id)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element channel id.", obj->conf_type);
		return WL_FAILED;
	}
	radio->channel_id = chan_id;
	return WL_SUCCESS;
}


int32_t set_radio_power(struct ap_cfg_info_st *obj, const uint8_t power)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element power.", obj->conf_type);
		return WL_FAILED;
	}
	radio->rf_power = power;
	return WL_SUCCESS;
}


int32_t set_radio_users_uplimit(struct ap_cfg_info_st *obj, const uint32_t user_limit)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no element users uplimit.", obj->conf_type);
		return WL_FAILED;
	}
	radio->rf_users_limit = user_limit;
	return WL_SUCCESS;
}


int32_t set_optim_bridge(struct ap_cfg_info_st *obj, const uint8_t bridge)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no vap bridge.", obj->conf_type);
		return WL_FAILED;
	}
	radio->vap_bridge = bridge;
	return WL_SUCCESS;
}


int32_t set_optim_mult_inspeed(struct ap_cfg_info_st *obj, const uint8_t inspeed)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no mult inspeed.", obj->conf_type);
		return WL_FAILED;
	}
	radio->mult_inspeed = inspeed;
	return WL_SUCCESS;
}


int32_t set_optim_sta_rate_limit(struct ap_cfg_info_st *obj, const uint8_t rate_limit)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no sta rate limit.", obj->conf_type);
		return WL_FAILED;
	}
	radio->sta_rate_limit = rate_limit;
	return WL_SUCCESS;
}


int32_t set_optim_mult_optim(struct ap_cfg_info_st *obj, const uint8_t mult_optim)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no mult_optim_enable.", obj->conf_type);
		return WL_FAILED;
	}
	radio->mult_optim_enable = mult_optim;
	return WL_SUCCESS;
}


int32_t set_optim_fairtime(struct ap_cfg_info_st *obj, const uint8_t fairtime)
{
	assert(obj);
	struct radio_cfg_st *radio = 
		(struct radio_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (obj->conf_type != CONF_RADIO)
	{
		LOG_WARN("CONF_TYPE(%d) has no mult_optim_enable.", obj->conf_type);
		return WL_FAILED;
	}
	radio->fairtime_enable = fairtime;
	return WL_SUCCESS;
}

int32_t set_wlan_rf_type(struct ap_cfg_info_st *obj, const uint8_t rf_type)
{
	assert(obj);
	struct wlan_cfg_st *wlan = 
		(struct wlan_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	if (rf_type < RF_2G || rf_type > RF_ALL)
	{
		LOG_WARN("RF_TYPE(%d) invalid.", rf_type);
		return WL_FAILED;
	}
	if (obj->conf_type != CONF_WLAN)
	{
		LOG_WARN("CONF_TYPE(%d) has no element rf type.", obj->conf_type);
		return WL_FAILED;
	}
	obj->rf_type = rf_type;
	wlan->rf_type = rf_type;
	return WL_SUCCESS;
}



int32_t set_wlan_id(struct ap_cfg_info_st *obj, const uint16_t wlan_id)
{
	assert(obj);
	struct wlan_cfg_st *wlan = 
		(struct wlan_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	
	if (obj->conf_type != CONF_WLAN)
	{
		LOG_WARN("CONF_TYPE(%d) has no element wlan_id.", obj->conf_type);
		return WL_FAILED;
	}
	wlan->wlan_id = wlan_id;
	return WL_SUCCESS;
}


int32_t set_wlan_hide_ssid(struct ap_cfg_info_st *obj, const uint8_t hide_enable)
{
	assert(obj);
	struct wlan_cfg_st *wlan = 
		(struct wlan_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	
	if (obj->conf_type != CONF_WLAN)
	{
		LOG_WARN("CONF_TYPE(%d) has no element hide.", obj->conf_type);
		return WL_FAILED;
	}
	wlan->hide = hide_enable;
	return WL_SUCCESS;
}



int32_t set_wlan_ssid(struct ap_cfg_info_st *obj, const char* ssid)
{
	assert(obj);
	struct wlan_cfg_st *wlan = 
		(struct wlan_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	
	if (obj->conf_type != CONF_WLAN)
	{
		LOG_WARN("CONF_TYPE(%d) has no element ssid.", obj->conf_type);
		return WL_FAILED;
	}
	safe_strncpy(wlan->ssid, ssid, SSID_MAX_SIZE);
	return WL_SUCCESS;
}



int32_t set_wlan_enable(struct ap_cfg_info_st *obj, const uint8_t enable)
{
	assert(obj);
	struct wlan_cfg_st *wlan = 
		(struct wlan_cfg_st*)((uint8_t*)obj + CONF_HEADER_SIZE);
	
	if (obj->conf_type != CONF_WLAN)
	{
		LOG_WARN("CONF_TYPE(%d) has no element hide.", obj->conf_type);
		return WL_FAILED;
	}
	wlan->wlan_enable = enable;
	return WL_SUCCESS;
}



/**
* @brief		参数有效性监测
* @param 	
* @param 	
* @return	
* @remark null
* @see
* @author tgb
**/
static int32_t config_obj_pars_check(CONFIG_CATEGORY conf_type)
{
	int32_t  ret = WL_SUCCESS;
	switch(conf_type)
	{
		case CONF_BASIC:
		case CONF_RADIO:
		case CONF_WLAN:
			ret = WL_SUCCESS;
			break;
		default:
			ret = WL_FAILED;
			break;
	}
	return ret;
}


/**
* @brief		依据指定类型，创建对象
* @param 	
* @param 	
* @return	
* @remark null
* @see
* @author tgb
**/
struct ap_cfg_info_st *create_config_obj(CONFIG_CATEGORY conf_type)
{
	uint8_t *obj = NULL;
	uint16_t body_len = 0, header_len = 0;
	struct ap_cfg_info_st *info = NULL;
	
	if (config_obj_pars_check(conf_type) == WL_FAILED)
	{
		LOG_WARN("conf_type(%d)  is invalid.", conf_type);
		return NULL;
	}
	header_len = sizeof(struct ap_cfg_info_st);
	switch(conf_type)
	{
		case CONF_BASIC:
			body_len = sizeof(struct ap_basic_cfg_st);
			break;

		case CONF_RADIO:
			body_len = sizeof(struct radio_cfg_st);
			break;

		case CONF_WLAN:
			body_len = sizeof(struct wlan_cfg_st);
			break;
			
		default:
			body_len = 0;
			break;
	}
	if (body_len == 0)
	{
		LOG_WARN("Nonsupport conf_type:%d.", conf_type);
		return NULL;
	}
	obj = (uint8_t*)malloc(header_len + body_len);
	if (obj == NULL)
	{
		LOG_WARN("malloc failed.");
		return NULL;
	}
	memset(obj, 0, header_len + body_len);
	info = (struct ap_cfg_info_st*)obj;
	info->magic = WL_MGMT_MAGIC;
	info->conf_type = conf_type;
	info->rf_type = RF_2G;	/*默认是2G*/
	info->len = body_len;
	LOG_DEBUG("create obj(type:%d,header_len:%d,body_len:%d) success",
				conf_type, header_len, body_len);
	return info;
}



void free_config_obj(struct ap_cfg_info_st *obj)
{
	if (obj)
	{
		free(obj);
	}
}

/**
* @brief		获取对象链的最后一个对象，及整个对象链的长度
* @param 	
* @param 	
* @return	
* @remark null
* @see
* @author tgb
**/
static struct ap_cfg_info_st* get_last_config_obj(struct ap_cfg_info_st *obj, uint16_t *occupy)
{
	if (obj == NULL || occupy == NULL)
	{
		return NULL;
	}
	uint16_t total_len = 0, obj_idx = 0, obj_len = 0;
	struct ap_cfg_info_st *info = obj;
	LOG_DEBUG("FIRST_OBJ_ADDR:%p.", obj);
	while (info)
	{
		obj_len = CONF_HEADER_SIZE + info->len;
		total_len += obj_len;
		//LOG_DEBUG("SUBOBJ_ADDR(%d):%p, LEN:%u.", obj_idx, info, obj_len);
		if (info->more)
		{
			info = (struct ap_cfg_info_st*)((uint8_t*)info + obj_len);
			obj_idx ++;
		}
		else
		{
			break;
		}
	}
	*occupy = total_len;
	LOG_DEBUG("LAST_OBJ_ADDR:%p.", info);
	LOG_DEBUG("SUBOJB_CNT:%u, TOTAL_LEN:%u.", (obj_idx + 1), total_len);
	return info;
}


/**
* @brief		整合子对象成一个大的对象，并释放子对象
* @param 	
* @param 	
* @return	
* @remark	如果整合出现错误，则传入的子对象不予释放
* @see
* @author tgb
**/
struct ap_cfg_info_st *pack_config_objs(uint8_t obj_cnt,...)
{
	assert(obj_cnt >= 1);
	uint16_t header_len = 0, body_len = 0, offset = 0, *occupy = NULL, total_occupy = 0;
	uint8_t idx = 0, *objs = NULL;
	va_list vars;
	struct ap_cfg_info_st *obj = NULL, **obj_list = NULL, **last_obj_list = NULL, *info = NULL;
	obj_list = (struct ap_cfg_info_st**)malloc(obj_cnt * sizeof(struct ap_cfg_info_st*));
	last_obj_list = (struct ap_cfg_info_st**)malloc(obj_cnt * sizeof(struct ap_cfg_info_st*));
	occupy = (uint16_t*)malloc(obj_cnt * sizeof(uint16_t));
	if (obj_list == NULL || last_obj_list == NULL || occupy == NULL)
	{
		goto NO_MEM;
	}
	memset(obj_list, 0, obj_cnt * sizeof(struct ap_cfg_info_st*));
	memset(last_obj_list, 0, obj_cnt * sizeof(struct ap_cfg_info_st*));
	memset(occupy, 0, obj_cnt * sizeof(uint16_t));
	LOG_DEBUG("vars parsing start...");
	va_start(vars, obj_cnt);
	for (idx = 0; idx < obj_cnt; idx++)
	{
		obj = va_arg(vars, struct ap_cfg_info_st*);
		obj_list[idx] = obj;
		last_obj_list[idx] = obj;
		if (obj == NULL)
		{
			continue;
		}
		last_obj_list[idx] = get_last_config_obj(obj,&occupy[idx]);
		total_occupy += occupy[idx];
	}
	va_end(vars);
	//LOG_DEBUG("TOTAL_OCCUPY:%u, OBJ_CNT:%u.", total_occupy, obj_cnt);
	//LOG_DEBUG("vars parsing finish.");
	if (total_occupy == 0)
	{
		return NULL;
	}
	objs = (uint8_t*)malloc(total_occupy);
	if (objs == NULL)
	{
		goto NO_MEM;
	}
	/*所有对象队列中最后一个对象的more位置1*/
	for (idx = 0; idx < obj_cnt; idx++)
	{
		if (last_obj_list[idx])
		{
			last_obj_list[idx]->more = 1;
		}
	}
	/*最后一个对象队列中最后一个对象的more位置0*/
	for (idx = obj_cnt - 1; idx >= 0; idx--)
	{
		if (last_obj_list[idx])
		{
			last_obj_list[idx]->more = 0;
			break;
		}
	}
	/*拷贝所有子对象并释放子对象*/
	for (idx = 0; idx < obj_cnt; idx++)
	{
		obj = obj_list[idx];
		if (obj == NULL)
		{
			continue;
		}
		info = (struct ap_cfg_info_st*)(objs + offset);
		//LOG_DEBUG("OBJS_ADDR:%p, OFFSET:%d, OBJ_ADDR:%p, ", objs, offset, info);
		memcpy(info, obj, occupy[idx]);
		offset += occupy[idx];	
		free(obj);
	}

	SAFE_FREE(obj_list);
	SAFE_FREE(last_obj_list);
	SAFE_FREE(occupy);
	LOG_DEBUG("PACK OBJS SUCCESS.");
	return (struct ap_cfg_info_st*)objs;
NO_MEM:
	SAFE_FREE(obj_list);
	SAFE_FREE(last_obj_list);
	SAFE_FREE(occupy);
	LOG_WARN("PACK OBJS FAILED.");
	return NULL;
}


static void print_basic_cfg(const struct ap_basic_cfg_st *basic)
{
	assert(basic);
	LOG_DEBUG("*********ap basic info start**********\n");
	LOG_DEBUG("COUNTRY_CODE:%u.\n", basic->country_code);
	LOG_DEBUG("WORK_MODE:%u.\n", basic->work_mode);
	LOG_DEBUG("AP_DESC:%s.\n", basic->ap_desc);
	LOG_DEBUG("AP_DEVID:%"PRIu64".\n", basic->ap_devid);
	LOG_DEBUG("DEBUG_SW:%d.\n", basic->debug_enable);
	LOG_DEBUG("*********ap basic info end************\n");
}




static void print_radio_cfg(const struct radio_cfg_st *radio)
{
	assert(radio);

	LOG_DEBUG("*********radio info start**********\n");
	LOG_DEBUG("RF_TYPE:%u.\n", radio->rf_type);
	LOG_DEBUG("RF_ENABLE:%d.\n", radio->rf_enable);
	LOG_DEBUG("RF_MODE:%s.\n", radio->rf_mode);
	
	LOG_DEBUG("CHANNEL_WIDTH:%u.\n", radio->channel_width);
	LOG_DEBUG("CHANNEL_ID:%d.\n", radio->channel_id);
	LOG_DEBUG("RF_POWER:%d.\n", radio->rf_power);

	LOG_DEBUG("USER_LIMITS:%u.\n", radio->rf_users_limit);
	LOG_DEBUG("AMPDU_ENABLE:%d.\n", radio->band_ampdu);
	LOG_DEBUG("AMSDU_ENABLE:%d.\n", radio->band_amsdu);

	LOG_DEBUG("BEACON:%u.\n", radio->band_beacon);
	LOG_DEBUG("RF_ENABLE:%d.\n", radio->band_dtim);
	LOG_DEBUG("LAEDCODE:%d.\n", radio->band_leadcode);

	LOG_DEBUG("RETRANS_MAX:%u.\n", radio->band_remax);
	LOG_DEBUG("RTS:%d.\n", radio->band_rts);
	LOG_DEBUG("SHORTGI:%d.\n", radio->band_shortgi);
	LOG_DEBUG("VAP_BRIDGE:%u.\n", radio->vap_bridge);
	LOG_DEBUG("FAIRTIME_ENABLE:%d.\n", radio->fairtime_enable);
	LOG_DEBUG("MULT_INSPEED:%f.\n", radio->mult_inspeed);
	LOG_DEBUG("MULTI_OPTIM_ENALBE:%u.\n", radio->mult_optim_enable);
	LOG_DEBUG("STA_RATE_LIMIT:%f.\n", radio->sta_rate_limit);
	
	LOG_DEBUG("LD_ENABLE:%d.\n", radio->ld_enable);
	LOG_DEBUG("REPORT_RADIO_CYCLE:%u.\n", radio->report_radio_info_cycle);
	LOG_DEBUG("REPORT_STA_CYCLE:%d.\n", radio->report_sta_info_cycle);
	LOG_DEBUG("HYBRID_SCAN_CYCLE:%d.\n", radio->hybrid_scan_cycle);
	LOG_DEBUG("HYBRID_SCAN_TIME:%d.\n", radio->hybrid_scan_time);
	LOG_DEBUG("MONITOR_SCAN_CYCLE:%d.\n", radio->monitor_scan_cycle);
	LOG_DEBUG("MONITOR_SCAN_TIME:%d.\n", radio->monitor_scan_time);
	LOG_DEBUG("NORMAL_SCAN_CYCLE:%d.\n", radio->normal_scan_cycle);
	LOG_DEBUG("NORMAL_SCAN_TIME:%d.\n", radio->normal_scan_time);
	LOG_DEBUG("SCAN_CHANNELS:%d.\n", radio->scan_channels);

	LOG_DEBUG("**radio info end************\n");
}



static void print_wlan_cfg(const struct wlan_cfg_st *wlan)
{
	assert(wlan);
	LOG_DEBUG("*********wlan info start**********\n");
	LOG_DEBUG("RF_TYPE:%u.\n", wlan->rf_type);
	LOG_DEBUG("WLAN_ID:%d.\n", wlan->wlan_id);
	LOG_DEBUG("SSID_HIDE:%d.\n", wlan->hide);
	
	LOG_DEBUG("ENCRYPTION:%s.\n", wlan->encryption);
	LOG_DEBUG("WLAN_ID:%s.\n", wlan->password);
	LOG_DEBUG("SSID_SSID:%s.\n", wlan->ssid);
	LOG_DEBUG("WLAN_ENABLE:%d.\n", wlan->wlan_enable);

	LOG_DEBUG("**wlan info end************\n");
}

static void print_debug(const struct ap_cfg_info_st *cfg_info)
{
	if (cfg_info == NULL)
	{
		return;
	}
	uint8_t *info_body = (uint8_t*)cfg_info + CONF_HEADER_SIZE;
	switch(cfg_info->conf_type)
	{
		case CONF_BASIC:
			LOG_DEBUG("basic_addr:%p.", cfg_info);
			print_basic_cfg((struct ap_basic_cfg_st *)info_body);
			break;

		case CONF_RADIO:
			LOG_DEBUG("radio_addr:%p.", cfg_info);
			print_radio_cfg((struct radio_cfg_st*)info_body);
		  break;

		case CONF_WLAN:
			LOG_DEBUG("wlan_addr:%p.", cfg_info);
			print_wlan_cfg((struct wlan_cfg_st *)info_body);
		  break;

		 default:
		 	LOG_WARN("unrecognized data.");
		 	break;
	}

}


void display_config_objs(const struct ap_cfg_info_st *info)
{
	if (info == NULL)
	{
		return;
	}
	LOG_DEBUG("DISPLAY START....");
	const struct ap_cfg_info_st *t_info = info;
	while(t_info)
	{		
		LOG_DEBUG("CONF_TYPE:%d;ADDR:%p.", t_info->conf_type, t_info);
		print_debug(t_info);
		if (t_info->more)
		{	/*处理下一个配置项*/
			t_info = (struct ap_cfg_info_st *)((uint8_t*)t_info + CONF_HEADER_SIZE + t_info->len);
		}		
		else		
		{			
			t_info = NULL;
		}	
	}
	LOG_DEBUG("DISPLAY END....");
}



void wl_radio_uninit()
{
	uint8_t radio_idx = 0;
	struct wl_radio_st *radio = NULL;
	for (radio_idx = 0; radio_idx < AP_RF_MAX_SIZE; radio_idx++)
	{
		radio = &g_wl_mgmt.radio[radio_idx];
		if (radio->ioctl_sock != -1)
		{
			close(radio->ioctl_sock);
			radio->ioctl_sock = -1;
		}
	}
	LOG_INFO("wireless radion uninit success.");
}


static int32_t create_sock(struct wl_radio_st *radio)
{
	assert(radio);
	if (radio->ioctl_sock == -1)
	{
		radio->ioctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	
	if (radio->ioctl_sock == -1)
	{
		LOG_WARN("Create ioctl sock failed.");
		return WL_FAILED;
	}
	LOG_INFO("Create ioctl sock success.");
	return WL_SUCCESS;
}

int32_t wl_radio_init()
{
	#define RF_2G_SYMBOL	"2g"
	#define RF_5G_SYMBOL	"5g"	
	uint8_t radio_idx = 0;
	int32_t ret = WL_SUCCESS;
	struct wl_radio_st *radio = NULL;
	struct wl_vap_st *vap = NULL;
	for (radio_idx = 0; radio_idx < AP_RF_MAX_SIZE; radio_idx++)
	{
		radio = &g_wl_mgmt.radio[radio_idx];
		radio->ioctl_sock = -1;
		snprintf(radio->wifi_name, IF_NAME_LEN, RADIO_NAME_FORMAT, radio_idx); 
		radio->rf_type = (radio_idx == 0 ? RF_2G : RF_5G);
		radio->assist.is_used = 1;
		if (radio_probe(radio) == WL_FAILED)
		{
			radio->assist.is_support = 0;
			LOG_INFO("Unsupport %s(rf_type:%d).", radio->wifi_name, radio->rf_type);
			continue;
		}
		LOG_INFO("SUPPORT %s(rf_type:%d).", radio->wifi_name, radio->rf_type);
		radio->assist.is_support = 1;
		if (create_sock(radio) == WL_FAILED)
		{
			ret = WL_FAILED;
			break;
		}
	}
	if (ret == WL_SUCCESS)
	{
		LOG_INFO("wireless radio init success.");
	}
	else
	{
		LOG_WARN("wireles radio init failed.");
	}
	#undef RF_2G_SYMBOL 
	#undef RF_5G_SYMBOL 	
	return ret;
}


int32_t wirless_commit_init()
{
  if (init_flag)
  {
    return WL_SUCCESS;
  }
  SAFE_BEZERO(&g_wl_mgmt);
  if (wl_radio_init() == WL_FAILED)
  {
    goto ERROR;
  }
  LOG_INFO("wireless mgmt init success.");
  init_flag = 1;
  return WL_SUCCESS;
ERROR:
  LOG_WARN("wireless mgmt init failed.");
  wireless_commit_uninit();
 return WL_FAILED;
}


void  wireless_commit_uninit()
{
  if (init_flag == 0)
  {
	return;
  }
  wl_radio_uninit();
}

int32_t wl_set_log_cb(log_print print)
{
	g_log_print = print;
}