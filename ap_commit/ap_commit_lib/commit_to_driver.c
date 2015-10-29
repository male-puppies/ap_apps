#include <unistd.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <unistd.h>
#include "wireless.h"
#include "commit_to_driver.h"

extern struct wl_mgmt_st g_wl_mgmt;

/**
* @brief    	获取国家码
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int32_t get_current_countrycode(uint32_t *country_code)
{
	int32_t ret = WL_SUCCESS, i = 0;
	for (i = RF_2G; i <= RF_5G; i++)
	{
		struct wl_radio_st *radio = get_wl_radio(i);
		if (radio)
		{
			ret = atheros_get_countrycode(radio->ioctl_sock, radio->wifi_name,country_code);
	        LOG_INFO("result = %d. country_code = %u.", ret, *country_code);	
			break;
		}
	}
	return ret;
}


/**
* @brief    	设置国家码
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int32_t set_country_code(uint32_t country_code)
{
	int32_t ret = WL_SUCCESS, i = 0;
	for (i = RF_2G; i <= RF_5G; i++)
	{
		struct wl_radio_st *radio = get_wl_radio(i);
		if (radio)
		{
		    ret = atheros_set_countrycode(radio->ioctl_sock, radio->wifi_name,country_code);
	        LOG_INFO("result = %d. country_code = %u.", ret, country_code);	
			continue;
		}
	}
	return ret;
}


/**
* @brief    	重启系统
* @param
* @return	成功不会返回；失败，返回结果
* @remark 文件写会磁盘；立即重启系统
* @see
* @author tgb
**/
static void reboot_sys()
{
	int32_t ret = WL_SUCCESS;
	LOG_INFO("Config commit need system restart...");
	sync();
	ret = reboot(LINUX_REBOOT_CMD_RESTART);
	LOG_WARN("Reboot system failed for %s.", strerror(errno));
	exit(EXIT_FAILURE);
	return;
}


/**
* @brief    修改国家码. 该操作可能导致AP重启
**/
int32_t proc_country_update()
{
	int current_country_code = 0;   /*AP正在使用的国家码*/
	int new_country_code = g_wl_mgmt.basic_info.basic_cfg.country_code;
	
	if (get_current_countrycode(&current_country_code) == WL_FAILED)
		return set_country_code(new_country_code);

    if (current_country_code == new_country_code)
        return WL_SUCCESS;

    /*国家码变了, 而且不是第一次修改,则重启AP.*/
    if (check_launch_flag() != 0)
        reboot_sys();
        
	return set_country_code(new_country_code);
}


/**
* @brief   获取vap name
* @param
* @return
* @remark 
* @see
* @author tgb
**/
static int32_t get_if_name(uint8_t rf_type, char *if_name, uint8_t len)
{
	assert(if_name);
	#define GET_IFNAME_CMD_FORMAT "ifconfig -a | grep %s | head -n 1 | "\
									"awk '{print $1}' 2>&1"
	int32_t ret = WL_FAILED;
	char cmd_str[POPEN_CMD_LEN] = {0}, result[POPEN_RESULT_LEN] = {0};
	if (rf_type == RF_2G)
	{
		snprintf(cmd_str, POPEN_CMD_LEN, GET_IFNAME_CMD_FORMAT, VAP_NAME_2G_PREFIX);
	}
	else
	{
		snprintf(cmd_str, POPEN_CMD_LEN, GET_IFNAME_CMD_FORMAT, VAP_NAME_5G_PREFIX);
	}
	
	if (custom_popen(cmd_str, POPEN_CMD_LEN, 
					 result, IF_NAME_LEN - 1) == WL_FAILED)
	{
		ret = WL_FAILED;
	}
	LOG_DEBUG("result:%s." ,result);
	if (strlen(result) == (IF_NAME_LEN - 1))
	{
		safe_strncpy(if_name, result, len);
		LOG_DEBUG("if_name:%s." ,if_name);
		return WL_SUCCESS;
	}
	return ret;
}


/**
* @brief    RADIO MODE更新处理
* @param
* @return
* @remark 
* @see
* @author tgb
**/
int32_t proc_radio_mode_update()
{	
	struct wl_radio_st *radio = NULL;
	int32_t disablecoext, cwmenable, ret = WL_SUCCESS;
	char if_name[IF_NAME_LEN] = {0};
	char mode[RADIO_MODE_MAX_SIZE] = {0};
	char real_mode[RADIO_MODE_MAX_SIZE] = {0};

    int i;	
	for (i = RF_2G; i <= RF_5G; i++)
	{
		radio = get_wl_radio(i);
		if (radio == NULL)
			continue;

		ret = construct_mode(radio->rf_type, radio->radio_cfg.rf_mode, 
							radio->radio_cfg.channel_width, mode, 
							RADIO_MODE_MAX_SIZE);
		if (ret == WL_FAILED)
		{
			LOG_DEBUG("construct mode failed.");
			return ret;
		}		
		
		if (get_if_name(radio->rf_type, if_name, IF_NAME_LEN)== WL_FAILED)
			continue;
		ret = atheros_get_mode(radio->ioctl_sock, if_name, real_mode, RADIO_MODE_MAX_SIZE);
		if (ret == WL_FAILED)
			continue;

		LOG_DEBUG("cur_mode:%s, tar_mode:%s.", real_mode, mode);
		if (strcmp(mode, real_mode) && check_launch_flag())
            reboot_sys();
            
	}
	
	return ret;
}


int32_t proc_debug_sw_update()
{
    uint8_t i, debug_sw = 0; 
    struct wl_radio_st *radio = NULL;

    debug_sw = g_wl_mgmt.basic_info.basic_cfg.debug_enable;
    for (i = RF_2G; i <= RF_5G; i++)
    {
        radio = get_wl_radio(i);
        if (radio == NULL)
            continue;
        if (atheros_set_chan_use_sw(radio->ioctl_sock, 
            radio->wifi_name, debug_sw) == WL_FAILED)
            return WL_FAILED;
    }
    return WL_SUCCESS;
}


/**
* @brief	全量下发
* @param
* @return
* @remark null
* @see
* @author tgb
**/
int32_t full_commit_per_radio(struct wl_radio_st *radio)
{
	assert(radio);

	LOG_DEBUG("full commit of %s start...", radio->wifi_name);
	if (radio->assist.is_configured == 0)
	{
		LOG_WARN("%s has no configured.", radio->wifi_name);
		return WL_SUCCESS;
	}
	if (radio_down(radio) == WL_FAILED)
	{
		goto FAILED; 
	}
	if (destroy_all_vap(radio) == WL_FAILED)
	{
		goto FAILED; 
	}
	if (create_all_vap(radio) == WL_FAILED)
	{
		goto FAILED; 
	}
	if (set_all_vap(radio) == WL_FAILED)
	{
		goto FAILED; 
	}	
	if (radio_cfg_commit(radio) == WL_FAILED)
	{
		goto FAILED; 
	}
	if (set_radio_enable(radio) == WL_FAILED)
	{
		goto FAILED;
	}
	/*up/down vap需在radio之后*/
	if (set_all_vap_enable(radio) == WL_FAILED)
	{
		goto FAILED; 
	}	
	
	LOG_DEBUG("full commit success of %s end.", radio->wifi_name);
	return WL_SUCCESS;
FAILED:
	LOG_DEBUG("full commit failed of %s end.", radio->wifi_name);
	return WL_FAILED;
}


/**
* @brief		通过popen执行命令
* @param 	 cmd_str 需要执行的命令
* @param 	 
* @param 	 result执行后的结果
* @param 	 
* @return	 
* @remark null
* @see
* @author tgb
**/
int32_t custom_popen(const char *cmd_str, const uint16_t cmd_len, 
							char *result, const uint16_t len)
{
	assert(cmd_str);
	assert(result);
	
	FILE *cmd_stream = NULL;
	uint8_t read_cnt = 0;
    int32_t ret = WL_FAILED;

	cmd_stream = popen(cmd_str, "r");
	if (cmd_stream == NULL)
	{
		ret = WL_FAILED;
		LOG_WARN("Cmd:%s, Execute cmd failed.", cmd_str);
		goto OUT;
	}
	read_cnt = fread(result, sizeof(uint8_t), len, cmd_stream);
	LOG_DEBUG("Cmd:%s; (len:result):%d,%s.", cmd_str, read_cnt, result);
	if (read_cnt >= len || (read_cnt < len && feof(cmd_stream)))
	{
		ret = WL_SUCCESS;
	}
	
OUT:
	if (cmd_stream)
	{
		pclose(cmd_stream);
	}
    return ret;
} 
