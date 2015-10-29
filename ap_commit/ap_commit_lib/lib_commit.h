#ifndef _LIB_COMMIT_H_
#define _LIB_COMMIT_H_
#include <stdint.h>
#include <stdarg.h>
#include "commit_common.h"

/*libcommit.so使用方法：*/
/*1.wireless_commit_init初始化库*/
/*2.使用create_config_obj创建子对象（当前支持CONF_BASIC,CONF_RADIO,CONF_WLAN三种类型对象）*/
/*3.创建若干子对象后，使用pack_config_objs可将所有子对象打包成一个大的对象PACKED_OBJ*/
/*4.将PACKED_OBJ传递给config_deliver,生效配置*/
/*5.调用接口free_config_obj释放PACKED_OBJ*/
/*6.调用wireless_commit_uninit释放库资源*/
/*NOTE:display_config_objs可用于打印参数，验证参数设置是否符合预期*/
/*NOTE:set_xxx_xxx用于设置相应类别对象的相应元素*/


int32_t wl_set_log_cb(log_print print);



int32_t wirless_commit_init();

/**
* @brief 配置下发接口
* @param	 cfg_info
* @return
* @remark 		该接口提供给外部调用，用于接收外部配置
* @see
* @author tgb
**/
int32_t config_deliver(const struct ap_cfg_info_st  *cfg_info);

void  wireless_commit_uninit();

/**
* @brief		依据指定类型，创建对象
* @remark null
* @author tgb
**/
struct ap_cfg_info_st *create_config_obj(CONFIG_CATEGORY conf_type);

/**
* @brief		整合子对象成一个大的对象，并释放子对象
* @return	非NULL，表示构造成功，并成功释放子对象；NULL构造失败，子对象未释放
* @remark null
* @see
* @author tgb
**/
struct ap_cfg_info_st *pack_config_objs(uint8_t obj_cnt,...);

void free_config_obj(struct ap_cfg_info_st *obj);

void display_config_objs(const struct ap_cfg_info_st *info);

/*======================CONF_BASIC======================*/
int32_t set_basic_country_code(struct ap_cfg_info_st *obj, const int32_t country_code);

/**
* @brief		参数设置- 工作模式
* @param 	
* @param 	
* @return	
* @remark  normal, hybrid, monitor三种工作模式
* @see
* @author tgb
**/
int32_t set_basic_work_mode(struct ap_cfg_info_st *obj, const int8_t work_mode);

/**
* @brief		参数设置-ap描述
* @param 	
* @param 	
* @return	
* @remark  normal, hybrid, monitor三种工作模式
* @see
* @author tgb
**/
int32_t set_basic_ap_desc(struct ap_cfg_info_st *obj, const char* ap_desc);

int32_t set_basic_ap_devid(struct ap_cfg_info_st *obj, const uint64_t dev_id);


/*======================CONF_RADIO======================*/
int32_t set_radio_rf_type(struct ap_cfg_info_st *obj, const uint8_t rf_type);

int32_t set_radio_rf_enable(struct ap_cfg_info_st *obj, const uint8_t rf_enable);

int32_t set_radio_rf_mode(struct ap_cfg_info_st *obj, const char* rf_mode);

int32_t set_radio_channel_width(struct ap_cfg_info_st *obj, const uint8_t chan_width);

int32_t set_radio_channel_id(struct ap_cfg_info_st *obj, const uint8_t chan_id);

int32_t set_radio_power(struct ap_cfg_info_st *obj, const uint8_t power);

int32_t set_radio_users_uplimit(struct ap_cfg_info_st *obj, const uint32_t user_limit);


/*======================CONF_WLAN======================*/
/**
* @brief		参数设置-wlan支持的射频类型
* @param 	
* @param 	
* @return	
* @remark  
* @see
* @author tgb
**/
int32_t set_wlan_rf_type(struct ap_cfg_info_st *obj, const uint8_t rf_type);

int32_t set_wlan_id(struct ap_cfg_info_st *obj, const uint16_t wlan_id);

int32_t set_wlan_hide_ssid(struct ap_cfg_info_st *obj, const uint8_t hide_enable);

int32_t set_wlan_ssid(struct ap_cfg_info_st *obj, const char* ssid);


int32_t set_wlan_enable(struct ap_cfg_info_st *obj, const uint8_t enable);
#endif
