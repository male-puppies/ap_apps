#ifndef AP_NETLINK_H__
#define AP_NETLINK_H__

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <inttypes.h>

#ifndef NETLINK_WIRELESS
#define NETLINK_WIRELESS              (NETLINK_GENERIC + 10) //netlink协议号
#endif

#define NETLINK_GROUP_ID    1     /*使用的netlink组IP*/
#define MAX_PAYLOAD (128 * 1024)  /*最大缓存*/
#define IEEE80211_NWID_LEN 32
#define IEEE80211_ADDR_LEN 6

#pragma pack(push)

#pragma pack(4)


/*netlink上报信息子类型*/
typedef enum ugw_report_type
{
    UGW_NEIGHBOR_WLAN_INFO  = 1,    //扫描邻居AP信息上报
    UGW_ASSOC_STA_INFO      = 2,    //关联终端信息上报
    UGW_RADIO_INFO          = 3,    //RADIO状态信息上报
    UGW_STA_AUTH_NOTIF      = 4,    //终端认证通知
    UGW_SCAN_STA_INFO       = 5,    //扫描终端信息
}ugw_report_type_t;


/*netlink信息结构体*/
typedef struct ugw_netlink_data
{
    u_int32_t type;     //netlink消息类型
    u_int32_t subtype;  //进一步区分消息类型使用
    u_int32_t count;    //结构体个数
    u_int8_t  buf[0];   //结构体数组
}ugw_netlink_data_t;


/*相邻WLAN信息结构体*/
typedef struct ugw_nwlan_info_
{
    u_int64_t    ap_devid;
    u_int32_t    rf_type;
    u_int32_t    channel_id;
    u_int32_t    rssi;
    u_int32_t    wlan_id;
    u_int8_t     ssid[IEEE80211_NWID_LEN+1];   
    u_int8_t     bssid[IEEE80211_ADDR_LEN];

	/* add end */
}ugw_nwlan_info_t;

#define IP_LEN 4
/*关联终端信息结构体*/
typedef struct ugw_sta_info_
{
    u_int8_t       sta_mac[IEEE80211_ADDR_LEN];
    u_int8_t       ip[IP_LEN];
    u_int8_t       bssid[IEEE80211_ADDR_LEN];
    u_int8_t       ssid[IEEE80211_NWID_LEN+1];
    u_int32_t      rssi;
    u_int32_t      rf_type;         //接入频段 0表示2G,1表示5G,和radio rf_type一致
    u_int32_t      is_dualband;     //是否双频
    u_int32_t      channel_id;      //终端信道
    u_int32_t      avg_tx_bytes;    //平均上行吞吐量
    u_int32_t      avg_rx_bytes;    //平均下行吞吐量
}ugw_sta_info_t;


#define MODE_LEN  30//无线协议的字符串长度,11nght40
/*radio状态信息上报*/
typedef struct ugw_radio_info_
{
    u_int8_t    mode[MODE_LEN];
    u_int32_t   rf_type;       //0表示2G,1表示5G
    u_int32_t   channel_id;    
    u_int32_t   txpower;       //dBm
    u_int32_t   curr_users;    //当前用户数
    u_int32_t   channel_use;   //百分比 65表示65%
    int32_t     noise;         //噪声

}ugw_radio_info_t;


/*终端上下线消息上报*/
typedef struct ugw_sta_auth_notif_
{
    u_int8_t    sta_mac[IEEE80211_ADDR_LEN];//终端MAC地址
    u_int64_t   ap_devid;                   //终端接入的ap
    u_int32_t   rf_type;                    //0表示2G,1表示5G
    u_int32_t   curr_users;                 //radio当前总用户数
    u_int8_t    bssid[IEEE80211_ADDR_LEN];  //终端接入的vap bssid
    u_int32_t   wlan_id;                    //所属WLAN ID
    u_int32_t   action;                     //1表示认证成功 0表示解除认证
}ugw_sta_auth_notif_t;


#define FULL_AMOUNT_REPORT  1  //扫描终端全量上报
#define INCREMENT_REPORT    2  //扫描终端增量上报
/*扫描终端消息上报结构体*/
typedef struct ugw_scan_sta_info_
{
    u_int8_t    sta_mac[IEEE80211_ADDR_LEN];//终端MAC地址
    u_int8_t    rssi;                       //信噪比
    u_int8_t    rf_type;                    //频段类型
}ugw_scan_sta_info_t;


/*MAC地址字符串格式*/
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

/*AP ID字符串格式*/
#define DEVIDSTR "%02x%02x%02x%02x%02x%02x%02x%02x"


int ugw_netlink_sock();  //返回netlink sock
int ugw_netlink_init();   //使用前需要调用,只需一次
int ugw_netlink_destroy(); //程序退出调用一次
int ugw_netlink_sendmsg(void *data, int len); //非阻塞读消息
int ugw_netlink_recvmsg(void **data, int *len); //非阻塞收消息

inline const char* MAC_STR(const u_int8_t mac[]);

inline const char* PRINT_APID(u_int64_t devid);

#pragma pack(pop)

#endif  //~AP_NETLINK_H__