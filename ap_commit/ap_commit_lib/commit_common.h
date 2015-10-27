#ifndef _COMMIT_COMMON_H_
#define _COMMIT_COMMON_H_

typedef int (*log_print) ( const char * format, ... );


/*配置类别，便于分类解析,当前支持前3种类型*/
typedef enum CONFIG_CATEGORY_TAG
{
	CONF_BASIC	= 0,
	CONF_RADIO,
	CONF_WLAN,
	CONF_NETWORK,
	CONF_AC,
	CONF_OTHER,
	CONF_MAX,
}CONFIG_CATEGORY;

/*header+content 形式使用该结构*/
#define 	WL_MGMT_MAGIC	(0xF1F2F3F4)
struct ap_cfg_info_st
{
	uint32_t 			magic;			/*魔数，用于校验配置的有效性*/
	CONFIG_CATEGORY		conf_type;		/*配置类别*/
	uint8_t 			rf_type;		/*2g,5g,all--1,2,3*/
	int8_t				more;			/*是否有更多的配置:1,则在当前的位置向后偏移，继续解析；0，则停止解析*/
	uint16_t			len;			/*当前配置项的长度，配置项内容紧随结构体后面*/
//  int8_t				data[0];			/*数据*/
};

/*射频类型*/
typedef enum RF_TYPE_TAG
{
  RF_2G = 1,	/*2G*/
  RF_5G = 2,	/*5G*/
  RF_ALL = 3,	/*2G&5G*/
}RF_TYPE;

/*长度限制*/
#define 	IF_NAME_LEN           8		/*设备名长度，形如ath2xxx或者wifiX*/
#define 	IEEE80211_ADDR_LEN    6		/*mac地址长度*/
#define 	AP_DESC_MAX_SIZE	  32	/*ap描述的长度*/
#define 	ENCRYPT_MODE_SIZE     32	/*加密模式*/
#define		PASSWORD_MAX_SIZE 	  32	/*密码长度*/
#define 	RADIO_MODE_MAX_SIZE	  16	/*无线协议*/
#define		SSID_MAX_SIZE     	  32	/*ssid长度*/
#define 	CONF_HEADER_SIZE 	  sizeof(struct ap_cfg_info_st)	/*头部长度*/

#define 	POPEN_CMD_LEN	256
#define 	POPEN_RESULT_LEN 256

#define COMMIT_LIB_LAUNCH_FLAG			"/tmp/commit_lib_launch_flag"	/*启动标记*/


/*********ap基本配置相关*********/
/*ap工作模式*/
typedef enum WORK_MODE_TAG
{
  NORMAL_MODE = 0,
  HYBRID_MODE = 1,
  MONITOR_MODE = 2,
}WORK_MODE;


/*********radio配置相关*********/
/*信道带宽*/
typedef enum  BANDWIDTH_TYPE_TAG
{
  BANDWIDTH_AUTO = 0,
  BANDWIDTH_20,
  BANDWIDTH_40_MINUS,
  BANDWIDTH_40_PLUS,
}CHANNEL_WIDTH;

/*信道ID选择*/
typedef enum CHANNEL_ID_TAG
{
  CHANNEL_AUTO = 0,
  CHANNEL_001 = 1,
}CHANNEL_ID;

/*功率类型*/
typedef enum POWER_TYPE_TAG
{
  POWER_AUTO = 0,
  POWER_003 = 3,
}POWER_TYPE;

/*扫描参数配置*/
typedef enum SCAN_CHANNEL_SCALE_TAG
{
  RECOMMEND_CHANNELS = 0,
  COUNTRY_CHANNELS = 1,
  ALL_CHANNELS  = 2,
}SCAN_CHANNEL_SCALE;


/*********wlan配置相关*********/
/*加密类型*/
typedef enum ENTRYPT_STYLE_TAG
{
	WAP2 = 0,
	PSK = 1,
}ENTRYPT_STYLE;


/*****************其他配置相关********************/
/*网络配置*/
typedef enum IP_ACCQUIRE_STYLE_TAG
{
    STATIC_STYLE  = 1,
    DHCP_STYLE    = 2,
}IP_ACCQURE_STYLE;

/*ac配置*/
struct ac_cfg_st
{
  uint32_t ac_host;
  uint16_t ac_port;
};


/*AP基本配置*/
struct ap_basic_cfg_st
{
	int32_t	country_code;            /*国家码*/
	WORK_MODE	work_mode;
	char		ap_desc[AP_DESC_MAX_SIZE]; /*ap的描述*/
	uint64_t  	ap_devid;
	uint8_t     debug_enable;          /*调试开关*/
};

/*radio配置*/
struct radio_cfg_st
{
  /*无线基本参数*/
  RF_TYPE		rf_type;
  uint8_t 		rf_enable;
  char 			rf_mode[RADIO_MODE_MAX_SIZE];
  CHANNEL_WIDTH channel_width;
  uint8_t     	channel_id;
  uint8_t   		rf_power;
  int16_t      	rf_users_limit;
  uint8_t       band_ampdu;
  uint8_t       band_amsdu;
  uint16_t      band_beacon;
  uint16_t      band_dtim;
  uint8_t       band_leadcode;
  uint8_t       band_remax;
  uint16_t      band_rts;
  uint8_t       band_shortgi;
  /*无线优化参数*/
  uint8_t vap_bridge;
  uint8_t mult_inspeed;
  uint8_t sta_rate_limit;
  uint8_t mult_optim_enable;
  uint8_t fairtime_enable;

 
  uint8_t 		ld_enable;
  uint16_t  report_radio_info_cycle;
  uint16_t  report_sta_info_cycle;
  /*扫描参数*/
  uint16_t  hybrid_scan_cycle;
  uint16_t  hybrid_scan_time;
  uint16_t  monitor_scan_cycle;
  uint16_t  monitor_scan_time;
  uint16_t  normal_scan_cycle;
  uint16_t  normal_scan_time;
  SCAN_CHANNEL_SCALE scan_channels;
};

/*wlan配置*/
struct wlan_cfg_st
{
  RF_TYPE rf_type;
  uint32_t wlan_id;
  uint8_t   hide;
  char	    encryption[ENCRYPT_MODE_SIZE];
  char      password[PASSWORD_MAX_SIZE];
  char      ssid[SSID_MAX_SIZE];
  uint8_t   wlan_enable;
};

/*无线优化配置*/
struct wl_optim_cfg_st
{
	uint8_t vap_bridge;
	uint8_t mult_inspeed;
	uint8_t sta_rate_limit;
	uint8_t mult_optim_enable;
	uint8_t fairtime_enable;
};
#endif
