#ifndef AP_IOCTL_H__
#define AP_IOCTL_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/if_bridge.h>

#pragma pack(push)

#pragma pack(4)

#define CMD_LEN 256

#define    SIOC80211IFCREATE           (SIOCDEVPRIVATE+7)    /*创建VAP实体*/
#define    SIOC80211IFDESTROY          (SIOCDEVPRIVATE+8)    /*删除VAP实体*/

//扩展vap ioctl
#ifndef IEEE80211_IOCTL_UGW_EXT
#define IEEE80211_IOCTL_UGW_EXT	   (SIOCDEVPRIVATE+13)
#endif

//扩展radio ioctl
#ifndef SIOCUGWEXT
#define SIOCUGWEXT (SIOCDEVPRIVATE+9)
#endif

#define    ATH_PARAM_SHIFT             0x1000    /*ath  ioctl过滤位*/
#define    SPECIAL_PARAM_SHIFT         0x2000    /*ath  ioctl特殊过过滤位*/


/*
 * flags to be passed to ieee80211_vap_create function .
 */
#define IEEE80211_CLONE_BSSID           0x0001  /* allocate unique mac/bssid */
#define IEEE80211_CLONE_NOBEACONS       0x0002  /* don't setup beacon timers */
#define IEEE80211_CLONE_WDS             0x0004  /* enable WDS processing */
#define IEEE80211_CLONE_WDSLEGACY       0x0008  /* legacy WDS operation */
#define IEEE80211_PRIMARY_VAP           0x0010  /* primary vap */
#define IEEE80211_P2PDEV_VAP            0x0020  /* p2pdev vap */
#define IEEE80211_P2PGO_VAP             0x0040  /* p2p-go vap */
#define IEEE80211_P2PCLI_VAP            0x0080  /* p2p-client vap */
#define IEEE80211_CLONE_MACADDR         0x0100  /* create vap w/ specified mac/bssid */
#define IEEE80211_CLONE_MATADDR         0x0200  /* create vap w/ specified MAT addr */
#define IEEE80211_WRAP_VAP              0x0400  /* wireless repeater ap vap */


#define    IFDATA_TLV_BUF_LEN    (1 * 1024) 
#define    IEEE80211_ADDR_LEN    6    /*mac地址长度*/
#define    IEEE80211_NWID_LEN    32   /*SSID长度*/

//驱动中原有ioctl
#define	IEEE80211_IOCTL_SETPARAM		(SIOCIWFIRSTPRIV+0)
#define	IEEE80211_IOCTL_GETPARAM		(SIOCIWFIRSTPRIV+1)
#define	IEEE80211_IOCTL_SETKEY			(SIOCIWFIRSTPRIV+2)
#define	IEEE80211_IOCTL_SETWMMPARAMS	(SIOCIWFIRSTPRIV+3)
#define	IEEE80211_IOCTL_DELKEY			(SIOCIWFIRSTPRIV+4)
#define	IEEE80211_IOCTL_GETWMMPARAMS	(SIOCIWFIRSTPRIV+5)
#define	IEEE80211_IOCTL_SETMLME			(SIOCIWFIRSTPRIV+6)
#define	IEEE80211_IOCTL_GETCHANINFO		(SIOCIWFIRSTPRIV+7)
#define	IEEE80211_IOCTL_SETOPTIE		(SIOCIWFIRSTPRIV+8)
#define	IEEE80211_IOCTL_GETOPTIE		(SIOCIWFIRSTPRIV+9)
#define	IEEE80211_IOCTL_ADDMAC			(SIOCIWFIRSTPRIV+10)        /* Add ACL MAC Address */
#define	IEEE80211_IOCTL_DELMAC			(SIOCIWFIRSTPRIV+12)        /* Del ACL MAC Address */
#define	IEEE80211_IOCTL_GETCHANLIST		(SIOCIWFIRSTPRIV+13)
#define	IEEE80211_IOCTL_SETCHANLIST		(SIOCIWFIRSTPRIV+14)
#define IEEE80211_IOCTL_KICKMAC			(SIOCIWFIRSTPRIV+15)
#define	IEEE80211_IOCTL_CHANSWITCH		(SIOCIWFIRSTPRIV+16)
#define	IEEE80211_IOCTL_GETMODE			(SIOCIWFIRSTPRIV+17)
#define	IEEE80211_IOCTL_SETMODE			(SIOCIWFIRSTPRIV+18)
#define IEEE80211_IOCTL_GET_APPIEBUF	(SIOCIWFIRSTPRIV+19)
#define IEEE80211_IOCTL_SET_APPIEBUF	(SIOCIWFIRSTPRIV+20)
#define IEEE80211_IOCTL_SET_ACPARAMS	(SIOCIWFIRSTPRIV+21)
#define IEEE80211_IOCTL_FILTERFRAME		(SIOCIWFIRSTPRIV+22)
#define IEEE80211_IOCTL_SET_RTPARAMS	(SIOCIWFIRSTPRIV+23)
#define IEEE80211_IOCTL_DBGREQ	        (SIOCIWFIRSTPRIV+24)
#define IEEE80211_IOCTL_SEND_MGMT		(SIOCIWFIRSTPRIV+26)
#define IEEE80211_IOCTL_SET_MEDENYENTRY (SIOCIWFIRSTPRIV+27)
#define IEEE80211_IOCTL_CHN_WIDTHSWITCH (SIOCIWFIRSTPRIV+28)
#define IEEE80211_IOCTL_GET_MACADDR		(SIOCIWFIRSTPRIV+29)        /* Get ACL List */
#define IEEE80211_IOCTL_SET_HBRPARAMS	(SIOCIWFIRSTPRIV+30)
#define IEEE80211_IOCTL_SET_RXTIMEOUT	(SIOCIWFIRSTPRIV+31)

/*ieee80211_iwpriv_cmd---ieee80211_iwpriv的子命令号*/
enum    
{
	IEEE80211_PARAM_TURBO		= 1,	/* turbo mode */
	IEEE80211_PARAM_MODE		= 2,	/* phy mode (11a, 11b, etc.) */
	IEEE80211_PARAM_AUTHMODE	= 3,	/* authentication mode */
	IEEE80211_PARAM_PROTMODE	= 4,	/* 802.11g protection */
	IEEE80211_PARAM_MCASTCIPHER	= 5,	/* multicast/default cipher */
	IEEE80211_PARAM_MCASTKEYLEN	= 6,	/* multicast key length */
	IEEE80211_PARAM_UCASTCIPHERS	= 7,	/* unicast cipher suites */
	IEEE80211_PARAM_UCASTCIPHER	= 8,	/* unicast cipher */
	IEEE80211_PARAM_UCASTKEYLEN	= 9,	/* unicast key length */
	IEEE80211_PARAM_WPA		= 10,	/* WPA mode (0,1,2) */
	IEEE80211_PARAM_ROAMING		= 12,	/* roaming mode */
	IEEE80211_PARAM_PRIVACY		= 13,	/* privacy invoked */
	IEEE80211_PARAM_COUNTERMEASURES	= 14,	/* WPA/TKIP countermeasures */
	IEEE80211_PARAM_DROPUNENCRYPTED	= 15,	/* discard unencrypted frames */
	IEEE80211_PARAM_DRIVER_CAPS	= 16,	/* driver capabilities */
	IEEE80211_PARAM_MACCMD		= 17,	/* MAC ACL operation */
	IEEE80211_PARAM_WMM		= 18,	/* WMM mode (on, off) */
	IEEE80211_PARAM_HIDESSID	= 19,	/* hide SSID mode (on, off) */
	IEEE80211_PARAM_APBRIDGE	= 20,	/* AP inter-sta bridging */
	IEEE80211_PARAM_KEYMGTALGS	= 21,	/* key management algorithms */
	IEEE80211_PARAM_RSNCAPS		= 22,	/* RSN capabilities */
	IEEE80211_PARAM_INACT		= 23,	/* station inactivity timeout */
	IEEE80211_PARAM_INACT_AUTH	= 24,	/* station auth inact timeout */
	IEEE80211_PARAM_INACT_INIT	= 25,	/* station init inact timeout */
	IEEE80211_PARAM_DTIM_PERIOD	= 28,	/* DTIM period (beacons) */
	IEEE80211_PARAM_BEACON_INTERVAL	= 29,	/* beacon interval (ms) */
	IEEE80211_PARAM_DOTH		= 30,	/* 11.h is on/off */
	IEEE80211_PARAM_PWRTARGET	= 31,	/* Current Channel Pwr Constraint */
	IEEE80211_PARAM_GENREASSOC	= 32,	/* Generate a reassociation request */
	IEEE80211_PARAM_COMPRESSION	= 33,	/* compression */
	IEEE80211_PARAM_FF		= 34,	/* fast frames support */
	IEEE80211_PARAM_XR		= 35,	/* XR support */
	IEEE80211_PARAM_BURST		= 36,	/* burst mode */
	IEEE80211_PARAM_PUREG		= 37,	/* pure 11g (no 11b stations) */
	IEEE80211_PARAM_AR		= 38,	/* AR support */
	IEEE80211_PARAM_WDS		= 39,	/* Enable 4 address processing */
	IEEE80211_PARAM_BGSCAN		= 40,	/* bg scanning (on, off) */
	IEEE80211_PARAM_BGSCAN_IDLE	= 41,	/* bg scan idle threshold */
	IEEE80211_PARAM_BGSCAN_INTERVAL	= 42,	/* bg scan interval */
	IEEE80211_PARAM_MCAST_RATE	= 43,	/* Multicast Tx Rate */
	IEEE80211_PARAM_COVERAGE_CLASS	= 44,	/* coverage class */
	IEEE80211_PARAM_COUNTRY_IE	= 45,	/* enable country IE */
	IEEE80211_PARAM_SCANVALID	= 46,	/* scan cache valid threshold */
	IEEE80211_PARAM_ROAM_RSSI_11A	= 47,	/* rssi threshold in 11a */
	IEEE80211_PARAM_ROAM_RSSI_11B	= 48,	/* rssi threshold in 11b */
	IEEE80211_PARAM_ROAM_RSSI_11G	= 49,	/* rssi threshold in 11g */
	IEEE80211_PARAM_ROAM_RATE_11A	= 50,	/* tx rate threshold in 11a */
	IEEE80211_PARAM_ROAM_RATE_11B	= 51,	/* tx rate threshold in 11b */
	IEEE80211_PARAM_ROAM_RATE_11G	= 52,	/* tx rate threshold in 11g */
	IEEE80211_PARAM_UAPSDINFO	= 53,	/* value for qos info field */
	IEEE80211_PARAM_SLEEP		= 54,	/* force sleep/wake */
	IEEE80211_PARAM_QOSNULL		= 55,	/* force sleep/wake */
	IEEE80211_PARAM_PSPOLL		= 56,	/* force ps-poll generation (sta only) */
	IEEE80211_PARAM_EOSPDROP	= 57,	/* force uapsd EOSP drop (ap only) */
	IEEE80211_PARAM_MARKDFS		= 58,	/* mark a dfs interference channel when found */
	IEEE80211_PARAM_REGCLASS	= 59,	/* enable regclass ids in country IE */
	IEEE80211_PARAM_CHANBW		= 60,	/* set chan bandwidth preference */
	IEEE80211_PARAM_WMM_AGGRMODE	= 61,	/* set WMM Aggressive Mode */
	IEEE80211_PARAM_SHORTPREAMBLE	= 62, 	/* enable/disable short Preamble */
	IEEE80211_PARAM_BLOCKDFSCHAN	= 63, 	/* enable/disable use of DFS channels */
	IEEE80211_PARAM_CWM_MODE	= 64,	/* CWM mode */
	IEEE80211_PARAM_CWM_EXTOFFSET	= 65,	/* CWM extension channel offset */
	IEEE80211_PARAM_CWM_EXTPROTMODE	= 66,	/* CWM extension channel protection mode */
	IEEE80211_PARAM_CWM_EXTPROTSPACING = 67,/* CWM extension channel protection spacing */
	IEEE80211_PARAM_CWM_ENABLE	= 68,/* CWM state machine enabled */
	IEEE80211_PARAM_CWM_EXTBUSYTHRESHOLD = 69,/* CWM extension channel busy threshold */
	IEEE80211_PARAM_CWM_CHWIDTH	= 70,	/* CWM STATE: current channel width */
	IEEE80211_PARAM_SHORT_GI	= 71,	/* half GI */
	IEEE80211_PARAM_FAST_CC		= 72,	/* fast channel change */

	/*
	 * 11n A-MPDU, A-MSDU support
	 */
	IEEE80211_PARAM_AMPDU		= 73,	/* 11n a-mpdu support */
	IEEE80211_PARAM_AMPDU_LIMIT	= 74,	/* a-mpdu length limit */
	IEEE80211_PARAM_AMPDU_DENSITY	= 75,	/* a-mpdu density */
	IEEE80211_PARAM_AMPDU_SUBFRAMES	= 76,	/* a-mpdu subframe limit */
	IEEE80211_PARAM_AMSDU		= 77,	/* a-msdu support */
	IEEE80211_PARAM_AMSDU_LIMIT	= 78,	/* a-msdu length limit */

	IEEE80211_PARAM_COUNTRYCODE	= 79,	/* Get country code */
	IEEE80211_PARAM_TX_CHAINMASK	= 80,	/* Tx chain mask */
	IEEE80211_PARAM_RX_CHAINMASK	= 81,	/* Rx chain mask */
	IEEE80211_PARAM_RTSCTS_RATECODE	= 82,	/* RTS Rate code */
	IEEE80211_PARAM_HT_PROTECTION	= 83,	/* Protect traffic in HT mode */
	IEEE80211_PARAM_RESET_ONCE	= 84,	/* Force a reset */
	IEEE80211_PARAM_SETADDBAOPER	= 85,	/* Set ADDBA mode */
	IEEE80211_PARAM_TX_CHAINMASK_LEGACY = 86, /* Tx chain mask for legacy clients */
	IEEE80211_PARAM_11N_RATE	= 87,	/* Set ADDBA mode */
	IEEE80211_PARAM_11N_RETRIES	= 88,	/* Tx chain mask for legacy clients */
	IEEE80211_PARAM_DBG_LVL		= 89,	/* Debug Level for specific VAP */
	IEEE80211_PARAM_WDS_AUTODETECT	= 90,	/* Configurable Auto Detect/Delba for WDS mode */
	IEEE80211_PARAM_ATH_RADIO	= 91,	/* returns the name of the radio being used */
	IEEE80211_PARAM_IGNORE_11DBEACON = 92,	/* Don't process 11d beacon (on, off) */
	IEEE80211_PARAM_STA_FORWARD	= 93,	/* Enable client 3 addr forwarding */

	/*
	 * Mcast Enhancement support
	 */
	IEEE80211_PARAM_ME          = 94,   /* Set Mcast enhancement option: 0 disable, 1 tunneling, 2 translate  4 to disable snoop feature*/
	IEEE80211_PARAM_MEDUMP		= 95,	/* Dump the snoop table for mcast enhancement */
	IEEE80211_PARAM_MEDEBUG		= 96,	/* mcast enhancement debug level */
	IEEE80211_PARAM_ME_SNOOPLENGTH	= 97,	/* mcast snoop list length */
	IEEE80211_PARAM_ME_TIMER	= 98,	/* Set Mcast enhancement timer to update the snoop list, in msec */
	IEEE80211_PARAM_ME_TIMEOUT	= 99,	/* Set Mcast enhancement timeout for STA's without traffic, in msec */
	IEEE80211_PARAM_PUREN		= 100,	/* pure 11n (no 11bg/11a stations) */
	IEEE80211_PARAM_BASICRATES	= 101,	/* Change Basic Rates */
	IEEE80211_PARAM_NO_EDGE_CH	= 102,	/* Avoid band edge channels */
	IEEE80211_PARAM_WEP_TKIP_HT	= 103,	/* Enable HT rates with WEP/TKIP encryption */
	IEEE80211_PARAM_RADIO		= 104,	/* radio on/off */
	IEEE80211_PARAM_NETWORK_SLEEP	= 105,	/* set network sleep enable/disable */
	IEEE80211_PARAM_DROPUNENC_EAPOL	= 106,

	/*
	 * Headline block removal
	 */
	IEEE80211_PARAM_HBR_TIMER	= 107,
	IEEE80211_PARAM_HBR_STATE	= 108,

	/*
	 * Unassociated power consumpion improve
	 */
	IEEE80211_PARAM_SLEEP_PRE_SCAN	= 109,
	IEEE80211_PARAM_SCAN_PRE_SLEEP	= 110,
	IEEE80211_PARAM_VAP_IND		= 111,  /* Independent VAP mode for Repeater and AP-STA config */

	/* support for wapi: set auth mode and key */
	IEEE80211_PARAM_SETWAPI		= 112,
	IEEE80211_IOCTL_GREEN_AP_PS_ENABLE = 113,
	IEEE80211_IOCTL_GREEN_AP_PS_TIMEOUT = 114,
	IEEE80211_IOCTL_GREEN_AP_PS_ON_TIME = 115,
	IEEE80211_PARAM_WPS		= 116,
	IEEE80211_PARAM_RX_RATE		= 117,
	IEEE80211_PARAM_CHEXTOFFSET	= 118,
	IEEE80211_PARAM_CHSCANINIT	= 119,
	IEEE80211_PARAM_MPDU_SPACING	= 120,
	IEEE80211_PARAM_HT40_INTOLERANT	= 121,
	IEEE80211_PARAM_CHWIDTH		= 122,
	IEEE80211_PARAM_EXTAP		= 123,   /* Enable client 3 addr forwarding */
        IEEE80211_PARAM_COEXT_DISABLE    = 124,
	IEEE80211_PARAM_ME_DROPMCAST	= 125,	/* drop mcast if empty entry */
	IEEE80211_PARAM_ME_SHOWDENY	= 126,	/* show deny table for mcast enhancement */
	IEEE80211_PARAM_ME_CLEARDENY	= 127,	/* clear deny table for mcast enhancement */
	IEEE80211_PARAM_ME_ADDDENY	= 128,	/* add deny entry for mcast enhancement */
    IEEE80211_PARAM_GETIQUECONFIG = 129, /*print out the iQUE config*/
    IEEE80211_PARAM_CCMPSW_ENCDEC = 130,  /* support for ccmp s/w encrypt decrypt */
      
      /* Support for repeater placement */ 
    IEEE80211_PARAM_CUSTPROTO_ENABLE = 131,
    IEEE80211_PARAM_GPUTCALC_ENABLE  = 132,
    IEEE80211_PARAM_DEVUP            = 133,
    IEEE80211_PARAM_MACDEV           = 134,
    IEEE80211_PARAM_MACADDR1         = 135,
    IEEE80211_PARAM_MACADDR2         = 136, 
    IEEE80211_PARAM_GPUTMODE         = 137, 
    IEEE80211_PARAM_TXPROTOMSG       = 138,
    IEEE80211_PARAM_RXPROTOMSG       = 139,
    IEEE80211_PARAM_STATUS           = 140,
    IEEE80211_PARAM_ASSOC            = 141,
    IEEE80211_PARAM_NUMSTAS          = 142,
    IEEE80211_PARAM_STA1ROUTE        = 143,
    IEEE80211_PARAM_STA2ROUTE        = 144,
    IEEE80211_PARAM_STA3ROUTE        = 145, 
    IEEE80211_PARAM_STA4ROUTE        = 146, 
    IEEE80211_PARAM_TDLS_ENABLE      = 147,  /* TDLS support */
    IEEE80211_PARAM_SET_TDLS_RMAC    = 148,  /* Set TDLS link */
    IEEE80211_PARAM_CLR_TDLS_RMAC    = 149,  /* Clear TDLS link */
    IEEE80211_PARAM_TDLS_MACADDR1    = 150,
    IEEE80211_PARAM_TDLS_MACADDR2    = 151,
    IEEE80211_PARAM_TDLS_ACTION      = 152,  
#if  ATH_SUPPORT_AOW
    IEEE80211_PARAM_SWRETRIES                   = 153,
    IEEE80211_PARAM_RTSRETRIES                  = 154,
    IEEE80211_PARAM_AOW_LATENCY                 = 155,
    IEEE80211_PARAM_AOW_STATS                   = 156,
    IEEE80211_PARAM_AOW_LIST_AUDIO_CHANNELS     = 157,
    IEEE80211_PARAM_AOW_PLAY_LOCAL              = 158,
    IEEE80211_PARAM_AOW_CLEAR_AUDIO_CHANNELS    = 159,
    IEEE80211_PARAM_AOW_INTERLEAVE              = 160,
    IEEE80211_PARAM_AOW_ER                      = 161,
    IEEE80211_PARAM_AOW_PRINT_CAPTURE           = 162,
    IEEE80211_PARAM_AOW_ENABLE_CAPTURE          = 163,
    IEEE80211_PARAM_AOW_FORCE_INPUT             = 164,
    IEEE80211_PARAM_AOW_EC                      = 165,
    IEEE80211_PARAM_AOW_EC_FMAP                 = 166,
    IEEE80211_PARAM_AOW_ES                      = 167,
    IEEE80211_PARAM_AOW_ESS                     = 168,
    IEEE80211_PARAM_AOW_ESS_COUNT               = 169,
    IEEE80211_PARAM_AOW_ESTATS                  = 170,
    IEEE80211_PARAM_AOW_AS                      = 171,
    IEEE80211_PARAM_AOW_PLAY_RX_CHANNEL         = 172,
    IEEE80211_PARAM_AOW_SIM_CTRL_CMD            = 173,
    IEEE80211_PARAM_AOW_FRAME_SIZE              = 174,
    IEEE80211_PARAM_AOW_ALT_SETTING             = 175,
    IEEE80211_PARAM_AOW_ASSOC_ONLY              = 176,
    IEEE80211_PARAM_AOW_EC_RAMP                 = 177,
    IEEE80211_PARAM_AOW_DISCONNECT_DEVICE       = 178,
#endif  /* ATH_SUPPORT_AOW */
    IEEE80211_PARAM_PERIODIC_SCAN = 179,
#if ATH_SUPPORT_AP_WDS_COMBO
    IEEE80211_PARAM_NO_BEACON     = 180,  /* No beacon xmit on VAP */
#endif
    IEEE80211_PARAM_VAP_COUNTRY_IE   = 181, /* 802.11d country ie per vap */
    IEEE80211_PARAM_VAP_DOTH         = 182, /* 802.11h per vap */
    IEEE80211_PARAM_STA_QUICKKICKOUT = 183, /* station quick kick out */
    IEEE80211_PARAM_AUTO_ASSOC       = 184,
    IEEE80211_PARAM_RXBUF_LIFETIME   = 185, /* lifetime of reycled rx buffers */
    IEEE80211_PARAM_2G_CSA           = 186, /* 2.4 GHz CSA is on/off */
    IEEE80211_PARAM_WAPIREKEY_USK = 187,
    IEEE80211_PARAM_WAPIREKEY_MSK = 188,
    IEEE80211_PARAM_WAPIREKEY_UPDATE = 189,
#if ATH_SUPPORT_IQUE
    IEEE80211_PARAM_RC_VIVO          = 190, /* Use separate rate control algorithm for VI/VO queues */
#endif
    IEEE80211_PARAM_CLR_APPOPT_IE    = 191,  /* Clear Cached App/OptIE */
    IEEE80211_PARAM_SW_WOW           = 192,   /* wow by sw */
    IEEE80211_PARAM_QUIET_PERIOD    = 193,
    IEEE80211_PARAM_QBSS_LOAD       = 194,
    IEEE80211_PARAM_RRM_CAP         = 195,
    IEEE80211_PARAM_WNM_CAP         = 196,
#if UMAC_SUPPORT_WDS
    IEEE80211_PARAM_ADD_WDS_ADDR    = 197,  /* add wds addr */
#endif
#ifdef QCA_PARTNER_PLATFORM
    IEEE80211_PARAM_PLTFRM_PRIVATE = 198, /* platfrom's private ioctl*/
#endif

#if UMAC_SUPPORT_VI_DBG
    /* Support for Video Debug */
    IEEE80211_PARAM_DBG_CFG            = 199,
    IEEE80211_PARAM_DBG_NUM_STREAMS    = 200,
    IEEE80211_PARAM_STREAM_NUM         = 201,
    IEEE80211_PARAM_DBG_NUM_MARKERS    = 202,
    IEEE80211_PARAM_MARKER_NUM         = 203,
    IEEE80211_PARAM_MARKER_OFFSET_SIZE = 204,
    IEEE80211_PARAM_MARKER_MATCH       = 205,
    IEEE80211_PARAM_RXSEQ_OFFSET_SIZE  = 206,
    IEEE80211_PARAM_RX_SEQ_RSHIFT      = 207,
    IEEE80211_PARAM_RX_SEQ_MAX         = 208,
    IEEE80211_PARAM_RX_SEQ_DROP        = 209,
    IEEE80211_PARAM_TIME_OFFSET_SIZE   = 210,
    IEEE80211_PARAM_RESTART            = 211,
    IEEE80211_PARAM_RXDROP_STATUS      = 212,
#endif    
    IEEE80211_PARAM_TDLS_DIALOG_TOKEN  = 213,  /* Dialog Token of TDLS Discovery Request */
    IEEE80211_PARAM_TDLS_DISCOVERY_REQ = 214,  /* Do TDLS Discovery Request */
    IEEE80211_PARAM_TDLS_AUTO_ENABLE   = 215,  /* Enable TDLS auto setup */
    IEEE80211_PARAM_TDLS_OFF_TIMEOUT   = 216,  /* Seconds of Timeout for off table : TDLS_OFF_TABLE_TIMEOUT */
    IEEE80211_PARAM_TDLS_TDB_TIMEOUT   = 217,  /* Seconds of Timeout for teardown block : TD_BLOCK_TIMEOUT */
    IEEE80211_PARAM_TDLS_WEAK_TIMEOUT  = 218,  /* Seconds of Timeout for weak peer : WEAK_PEER_TIMEOUT */
    IEEE80211_PARAM_TDLS_RSSI_MARGIN   = 219,  /* RSSI margin between AP path and Direct link one */
    IEEE80211_PARAM_TDLS_RSSI_UPPER_BOUNDARY= 220,  /* RSSI upper boundary of Direct link path */
    IEEE80211_PARAM_TDLS_RSSI_LOWER_BOUNDARY= 221,  /* RSSI lower boundary of Direct link path */
    IEEE80211_PARAM_TDLS_PATH_SELECT        = 222,  /* Enable TDLS Path Select bewteen AP path and Direct link one */
    IEEE80211_PARAM_TDLS_RSSI_OFFSET        = 223,  /* RSSI offset of TDLS Path Select */
    IEEE80211_PARAM_TDLS_PATH_SEL_PERIOD    = 224,  /* Period time of Path Select */
#if ATH_SUPPORT_IBSS_DFS
    IEEE80211_PARAM_IBSS_DFS_PARAM     = 225,
#endif 
    IEEE80211_PARAM_TDLS_TABLE_QUERY        = 236,  /* Print Table info. of AUTO-TDLS */
#if ATH_SUPPORT_IBSS_NETLINK_NOTIFICATION
    IEEE80211_PARAM_IBSS_SET_RSSI_CLASS     = 237,	
    IEEE80211_PARAM_IBSS_START_RSSI_MONITOR = 238,
    IEEE80211_PARAM_IBSS_RSSI_HYSTERESIS    = 239,
#endif
#ifdef ATH_SUPPORT_TxBF
    IEEE80211_PARAM_TXBF_AUTO_CVUPDATE = 240,       /* Auto CV update enable*/
    IEEE80211_PARAM_TXBF_CVUPDATE_PER = 241,        /* per theshold to initial CV update*/
#endif
    IEEE80211_PARAM_MAXSTA              = 242,
    IEEE80211_PARAM_RRM_STATS               =243,
    IEEE80211_PARAM_RRM_SLWINDOW            =244,
    IEEE80211_PARAM_MFP_TEST    = 245,
    IEEE80211_PARAM_SCAN_BAND   = 246,                /* only scan channels of requested band */
#if ATH_SUPPORT_FLOWMAC_MODULE
    IEEE80211_PARAM_FLOWMAC            = 247, /* flowmac enable/disable ath0*/
#endif
#if CONFIG_RCPI        
    IEEE80211_PARAM_TDLS_RCPI_HI     = 248,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_RCPI_LOW    = 249,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_RCPI_MARGIN = 250,    /* RCPI params: hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_SET_RCPI    = 251,    /* RCPI params: set hi,lo threshold and margin */
    IEEE80211_PARAM_TDLS_GET_RCPI    = 252,    /* RCPI params: get hi,lo threshold and margin */
#endif  
    IEEE80211_PARAM_TDLS_PEER_UAPSD_ENABLE  = 253, /* Enable TDLS Peer U-APSD Power Save feature */   
    IEEE80211_PARAM_TDLS_QOSNULL            = 254, /* Send QOSNULL frame to remote peer */
    IEEE80211_PARAM_STA_PWR_SET_PSPOLL      = 255,  /* Set ips_use_pspoll flag for STA */
    IEEE80211_PARAM_NO_STOP_DISASSOC        = 256,  /* Do not send disassociation frame on stopping vap */
#if UMAC_SUPPORT_IBSS
    IEEE80211_PARAM_IBSS_CREATE_DISABLE = 257,      /* if set, it prevents IBSS creation */
#endif
#if ATH_SUPPORT_WIFIPOS
    IEEE80211_PARAM_WIFIPOS_TXCORRECTION = 258,      /* Set/Get TxCorrection */
    IEEE80211_PARAM_WIFIPOS_RXCORRECTION = 259,      /* Set/Get RxCorrection */
#endif
#if UMAC_SUPPORT_CHANUTIL_MEASUREMENT
    IEEE80211_PARAM_CHAN_UTIL_ENAB      = 260,
    IEEE80211_PARAM_CHAN_UTIL           = 261,      /* Get Channel Utilization value (scale: 0 - 255) */
#endif /* UMAC_SUPPORT_CHANUTIL_MEASUREMENT */
    IEEE80211_PARAM_DBG_LVL_HIGH        = 262, /* Debug Level for specific VAP (upper 32 bits) */
    IEEE80211_PARAM_PROXYARP_CAP        = 263, /* Enable WNM Proxy ARP feature */
    IEEE80211_PARAM_DGAF_DISABLE        = 264, /* Hotspot 2.0 DGAF Disable feature */
    IEEE80211_PARAM_L2TIF_CAP           = 265, /* Hotspot 2.0 L2 Traffic Inspection and Filtering */
    IEEE80211_PARAM_WEATHER_RADAR_CHANNEL = 266, /* weather radar channel selection is bypassed */
    IEEE80211_PARAM_SEND_DEAUTH           = 267,/* for sending deauth while doing interface down*/
    IEEE80211_PARAM_WEP_KEYCACHE          = 268,/* wepkeys mustbe in first fourslots in Keycache*/
#if ATH_SUPPORT_WPA_SUPPLICANT_CHECK_TIME    
    IEEE80211_PARAM_REJOINT_ATTEMP_TIME   = 269, /* Set the Rejoint time */
#endif
    IEEE80211_PARAM_WNM_SLEEP           = 270,      /* WNM-Sleep Mode */
    IEEE80211_PARAM_WNM_BSS_CAP         = 271,
    IEEE80211_PARAM_WNM_TFS_CAP         = 272,
    IEEE80211_PARAM_WNM_TIM_CAP         = 273,
    IEEE80211_PARAM_WNM_SLEEP_CAP       = 274,
    IEEE80211_PARAM_WNM_FMS_CAP         = 275,
    IEEE80211_PARAM_RRM_DEBUG           = 276, /* RRM debugging parameter */
    IEEE80211_PARAM_SET_TXPWRADJUST     = 277,
    IEEE80211_PARAM_TXRX_DBG              = 278,    /* show txrx debug info */
    IEEE80211_PARAM_VHT_MCS               = 279,    /* VHT MCS set */
    IEEE80211_PARAM_TXRX_FW_STATS         = 280,    /* single FW stat */
    IEEE80211_PARAM_TXRX_FW_MSTATS        = 281,    /* multiple FW stats */
    IEEE80211_PARAM_NSS                   = 282,    /* Number of Spatial Streams */
    IEEE80211_PARAM_LDPC                  = 283,    /* Support LDPC */
    IEEE80211_PARAM_TX_STBC               = 284,    /* Support TX STBC */
    IEEE80211_PARAM_RX_STBC               = 285,    /* Support RX STBC */
#if UMAC_SUPPORT_SMARTANTENNA
    IEEE80211_PARAM_SMARTANT_RETRAIN_THRESHOLD = 286,    /* number of iterations needed for training */
    IEEE80211_PARAM_SMARTANT_RETRAIN_INTERVAL = 287,    /* number of iterations needed for training */
    IEEE80211_PARAM_SMARTANT_RETRAIN_DROP = 288,    /* number of iterations needed for training */
#endif
#if UMAC_SUPPORT_TDLS
    IEEE80211_PARAM_TDLS_SET_OFF_CHANNEL    = 288,  /* Configuration of off channel operations */
    IEEE80211_PARAM_TDLS_SWITCH_TIME        = 289,  /* Time to perform channel switch to an off channel */
    IEEE80211_PARAM_TDLS_SWITCH_TIMEOUT     = 290,  /* User configured timeout value for switching to an off channel */
    IEEE80211_PARAM_TDLS_SEC_CHANNEL_OFFSET = 291,  
    IEEE80211_PARAM_TDLS_OFF_CHANNEL_MODE   = 292,
#endif
    IEEE80211_PARAM_APONLY                  = 293,
    IEEE80211_PARAM_TXRX_FW_STATS_RESET     = 294,
    IEEE80211_PARAM_TX_PPDU_LOG_CFG         = 295,  /* tx PPDU log cfg params */
    IEEE80211_PARAM_OPMODE_NOTIFY           = 296,  /* Op Mode Notification */
    IEEE80211_PARAM_NOPBN                   = 297, /* don't send push button notification */
    IEEE80211_PARAM_DFS_CACTIMEOUT          = 298, /* override CAC timeout */
    IEEE80211_PARAM_ENABLE_RTSCTS           = 299, /* Enable/disable RTS-CTS */

    IEEE80211_PARAM_MAX_AMPDU               = 300,   /* Set/Get rx AMPDU exponent/shift */
    IEEE80211_PARAM_VHT_MAX_AMPDU           = 301,   /* Set/Get rx VHT AMPDU exponent/shift */
    IEEE80211_PARAM_BCAST_RATE              = 302,   /* Setting Bcast DATA rate */
    IEEE80211_PARAM_VHT_MCSMAP              = 303,   /* Set VHT MCS MAP */
    IEEE80211_PARAM_PARENT_IFINDEX          = 304,   /* parent net_device ifindex for this VAP */
#if WDS_VENDOR_EXTENSION
    IEEE80211_PARAM_WDS_RX_POLICY           = 305,  /* Set/Get WDS rx filter policy for vendor specific WDS */
#endif
    IEEE80211_PARAM_ENABLE_OL_STATS         = 306,   /*Enables/Disables the 
                                                        stats in the Host and in the FW */
    IEEE80211_IOCTL_GREEN_AP_ENABLE_PRINT   = 307,  /* Enable/Disable Green-AP debug prints */
    IEEE80211_PARAM_RC_NUM_RETRIES          = 308,
    IEEE80211_PARAM_GET_ACS                 = 309,/* to get status of acs */
    IEEE80211_PARAM_GET_CAC                 = 310,/* to get status of CAC period */
    IEEE80211_PARAM_EXT_IFACEUP_ACS         = 311,  /* Enable external auto channel selection entity
                                                       at VAP init time */
    IEEE80211_PARAM_ONETXCHAIN              = 312,  /* force to tx with one chain for legacy client */
    IEEE80211_PARAM_DFSDOMAIN               = 313,  /* Get DFS Domain */
    IEEE80211_PARAM_SCAN_CHAN_EVENT         = 314,  /* Enable delivery of Scan Channel Events during
                                                       802.11 scans (11ac offload, and IEEE80211_M_HOSTAP
                                                       mode only). */
    IEEE80211_PARAM_DESIRED_CHANNEL         = 315,  /* Get desired channel corresponding to desired
                                                       PHY mode */
    IEEE80211_PARAM_DESIRED_PHYMODE         = 316,  /* Get desired PHY mode */
    IEEE80211_PARAM_SEND_ADDITIONAL_IES     = 317,  /* Control sending of additional IEs to host */
    IEEE80211_PARAM_START_ACS_REPORT        = 318,  /* to start acs scan report */
    IEEE80211_PARAM_MIN_DWELL_ACS_REPORT    = 319,  /* min dwell time for  acs scan report */
    IEEE80211_PARAM_MAX_DWELL_ACS_REPORT    = 320,  /* max dwell time for  acs scan report */
    IEEE80211_PARAM_ACS_CH_HOP_LONG_DUR     = 321,  /* channel long duration timer used in acs */
    IEEE80211_PARAM_ACS_CH_HOP_NO_HOP_DUR   = 322,  /* No hopping timer used in acs */
    IEEE80211_PARAM_ACS_CH_HOP_CNT_WIN_DUR  = 323,  /* counter window timer used in acs */
    IEEE80211_PARAM_ACS_CH_HOP_NOISE_TH     = 324,  /* Noise threshold used in acs channel hopping */
    IEEE80211_PARAM_ACS_CH_HOP_CNT_TH       = 325,  /* counter threshold used in acs channel hopping */
    IEEE80211_PARAM_ACS_ENABLE_CH_HOP       = 326,  /* Enable/Disable acs channel hopping */
    IEEE80211_PARAM_SET_CABQ_MAXDUR         = 327,  /* set the max tx percentage for cabq */
    IEEE80211_PARAM_256QAM_2G               = 328,  /* 2.4 GHz 256 QAM support */
#if ATH_DEBUG
    IEEE80211_PARAM_OFFCHAN_TX              = 329,  /* testing offchan transmission */
#endif
    IEEE80211_PARAM_MAX_SCANENTRY           = 330,  /* MAX scan entry */
    IEEE80211_PARAM_SCANENTRY_TIMEOUT       = 331,  /* Scan entry timeout value */
    IEEE80211_PARAM_PURE11AC                = 332,  /* pure 11ac(no 11bg/11a/11n stations) */
#if UMAC_VOW_DEBUG
    IEEE80211_PARAM_VOW_DBG_ENABLE  = 333,  /*Enable VoW debug*/
#endif
    
    IEEE80211_PARAM_WIFI_LOG              = 350,
    IEEE80211_PARAM_SCAN_STATE            = 351,
    IEEE80211_PARAM_BSS_TO_CW20           = 352,/*使用20MHz固定带宽的开关*/
    IEEE80211_PARAM_VIP_QUEUE             = 353,/*是否使用关键帧硬件队列，提高某些帧优先级的开关 */
    IEEE80211_PARAM_FAIRTIME              = 354,/*时间公平开关*/
    IEEE80211_PARAM_MCAST_RATIO           = 355,
    IEEE80211_PARAM_WEIGHT_ENABLE         = 356,
    IEEE80211_PARAM_VAP_WEIGHT            = 357,
    IEEE80211_PARAM_QOS_ENABLE            = 358,
    IEEE80211_PARAM_SUBCHAN_ENABLE        = 359,
    IEEE80211_PARAM_ASSOC_TIMEOUT         = 360,/*关联查询超时时间*/
    IEEE80211_PARAM_QOS_LOG               = 361,
    IEEE80211_PARAM_QOS_DEBUG             = 362,   
    IEEE80211_PARAM_QOS_MAX_WBUF          = 363,
    IEEE80211_PARAM_QOS_DEQ_MAX           = 364,
    IEEE80211_PARAM_GET_BALANCE_SW        = 365,/*负载均衡开关,只读*/   
    IEEE80211_PARAM_GET_RT_SNR_DIFF       = 366,/*实时上报终端信噪比变化阀值, 只读*/
    IEEE80211_PARAM_GET_RT_INTERVAL       = 367,/*负载均衡上报终端周期,只读*/
    IEEE80211_PARAM_TRACE_DHCP            = 368,/*DHCP状态跟踪，默认关闭*/
    IEEE80211_PARAM_SHOW_TX_STEP          = 369,/*TX路径打印*/
    IEEE80211_PARAM_SHOW_RX_STEP          = 370,/*RX路径打印*/
    IEEE80211_PARAM_DEBUG_TID_CHECK       = 371,/*TID调试开关*/
    IEEE80211_PARAM_LOG_LIMIT             = 372,/*驱动日志限速开关*/
    IEEE80211_PARAM_VAP_SUBCHAN           = 373,/*add by ygx 获取子通道信息*/
    IEEE80211_PARAM_VAP_MODE              = 374,/*vap模式，比如idle, normal等*/
    IEEE80211_PARAM_DBG_STATS             = 375, /*统计数据调试*/
    IEEE80211_PARAM_DHCP_BYPASS			  = 381, /*dhcp bypass 开关,added by tanguangbao */
};




/*
** Enumeration of parameter IDs
** This is how the external users refer to specific parameters, which is
** why it's defined in the external interface
*/

typedef enum {
    ATH_PARAM_TXCHAINMASK           =1,
    ATH_PARAM_RXCHAINMASK           =2,
    ATH_PARAM_TXCHAINMASKLEGACY     =3,
    ATH_PARAM_RXCHAINMASKLEGACY     =4,
    ATH_PARAM_CHAINMASK_SEL         =5,
    ATH_PARAM_AMPDU                 =6,
    ATH_PARAM_AMPDU_LIMIT           =7,
    ATH_PARAM_AMPDU_SUBFRAMES       =8,
    ATH_PARAM_AGGR_PROT             =9,
    ATH_PARAM_AGGR_PROT_DUR         =10,
    ATH_PARAM_AGGR_PROT_MAX         =11,
    ATH_PARAM_TXPOWER_LIMIT2G       =12,
    ATH_PARAM_TXPOWER_LIMIT5G       =13,
    ATH_PARAM_TXPOWER_OVERRIDE      =14,
    ATH_PARAM_PCIE_DISABLE_ASPM_WK  =15,
    ATH_PARAM_PCID_ASPM             =16,
    ATH_PARAM_BEACON_NORESET        =17,
    ATH_PARAM_CAB_CONFIG            =18,
    ATH_PARAM_ATH_DEBUG             =19,
    ATH_PARAM_ATH_TPSCALE           =20,
    ATH_PARAM_ACKTIMEOUT            =21,
#ifdef ATH_RB
    ATH_PARAM_RX_RB                 =22,
    ATH_PARAM_RX_RB_DETECT          =23,
    ATH_PARAM_RX_RB_TIMEOUT         =24,
    ATH_PARAM_RX_RB_SKIPTHRESH      =25,
#endif
    ATH_PARAM_AMSDU_ENABLE          =26,
#if ATH_SUPPORT_IQUE
    ATH_PARAM_RETRY_DURATION        =27,
    ATH_PARAM_HBR_HIGHPER           =28,
    ATH_PARAM_HBR_LOWPER            =29,
#endif
    ATH_PARAM_RX_STBC               =30,
    ATH_PARAM_TX_STBC               =31,
    ATH_PARAM_LDPC                  =32,
    ATH_PARAM_LIMIT_LEGACY_FRM      =33,
    ATH_PARAM_TOGGLE_IMMUNITY       =34,
    ATH_PARAM_WEP_TKIP_AGGR_TX_DELIM    =35,
    ATH_PARAM_WEP_TKIP_AGGR_RX_DELIM    =36,
    ATH_PARAM_GPIO_LED_CUSTOM       =37,
    ATH_PARAM_SWAP_DEFAULT_LED      =38,
#if defined(ATH_SUPPORT_WIRESHARK)
    ATH_PARAM_TAPMONITOR            =39,
#endif
#if ATH_SUPPORT_VOWEXT
    ATH_PARAM_VOWEXT                =40,
    /*ATH_PARAM_RCA                   =41,*/ /* rate control and 
                                       aggregation parameters */
	ATH_PARAM_VSP_ENABLE            =42,
	ATH_PARAM_VSP_THRESHOLD         =43,
	ATH_PARAM_VSP_EVALINTERVAL      =44,
#endif
#if ATH_VOW_EXT_STATS
    ATH_PARAM_VOWEXT_STATS          =45,
#endif
#ifdef VOW_TIDSCHED
    ATH_PARAM_TIDSCHED              =46,
    ATH_PARAM_TIDSCHED_VOQW         =47,
    ATH_PARAM_TIDSCHED_VIQW         =48,
    ATH_PARAM_TIDSCHED_BKQW         =49,
    ATH_PARAM_TIDSCHED_BEQW         =50,
    ATH_PARAM_TIDSCHED_VOTO         =51,
    ATH_PARAM_TIDSCHED_VITO         =52,
    ATH_PARAM_TIDSCHED_BKTO         =53,
    ATH_PARAM_TIDSCHED_BETO         =54,
#endif
#if  ATH_SUPPORT_AOW
    ATH_PARAM_SW_RETRY_LIMIT        =55,
    ATH_PARAM_AOW_LATENCY           =56,
    ATH_PARAM_AOW_STATS             =57,
    ATH_PARAM_AOW_LIST_AUDIO_CHANNELS   =58,
    ATH_PARAM_AOW_PLAY_LOCAL            =59,
    ATH_PARAM_AOW_CLEAR_AUDIO_CHANNELS  =60,
    ATH_PARAM_AOW_ER                    =61,
    ATH_PARAM_AOW_EC                    =62,
    ATH_PARAM_AOW_EC_FMAP               =63,
#endif  /* ATH_SUPPORT_AOW */
    /*Thresholds for interrupt mitigation*/
    ATH_PARAM_RIMT_FIRST                =64,
    ATH_PARAM_RIMT_LAST                 =65,
    ATH_PARAM_TIMT_FIRST                =66,
    ATH_PARAM_TIMT_LAST                 =67,
#ifdef VOW_LOGLATENCY
    ATH_PARAM_LOGLATENCY                =68,
#endif
    ATH_PARAM_TXBF_SW_TIMER             =69,
    ATH_PARAM_PHYRESTART_WAR            =70,
    ATH_PARAM_CHANNEL_SWITCHING_TIME_USEC    =71,
    ATH_PARAM_KEYSEARCH_ALWAYS_WAR      = 72,
#ifdef ATH_SUPPORT_DYN_TX_CHAINMASK
    ATH_PARAM_DYN_TX_CHAINMASK          =73,
#endif /* ATH_SUPPORT_DYN_TX_CHAINMASK */
#if ATH_SUPPORT_VOWEXT
    ATH_PARAM_VSP_STATS                 =74,
    ATH_PARAM_VSP_STATSCLR              =75,
#endif

#if UMAC_SUPPORT_SMARTANTENNA
    ATH_PARAM_SMARTANTENNA          = 76,
#endif    
#if ATH_SUPPORT_AGGR_BURST
    ATH_PARAM_AGGR_BURST                =77,
    ATH_PARAM_AGGR_BURST_DURATION       =78,
#endif

#if ATH_SUPPORT_FLOWMAC_MODULE
    ATH_PARAM_FLOWMAC                   =79,
#endif
    ATH_PARAM_BCN_BURST                 =80,
#if ATH_ANI_NOISE_SPUR_OPT
    ATH_PARAM_NOISE_SPUR_OPT            =81,
#endif
    ATH_PARAM_DCS_ENABLE                =82,
#if UMAC_SUPPORT_PERIODIC_PERFSTATS
    ATH_PARAM_PRDPERFSTAT_THRPUT_ENAB  = 83,
    ATH_PARAM_PRDPERFSTAT_THRPUT_WIN   = 84,
    ATH_PARAM_PRDPERFSTAT_THRPUT       = 85,
    ATH_PARAM_PRDPERFSTAT_PER_ENAB     = 86,
    ATH_PARAM_PRDPERFSTAT_PER_WIN      = 87,
    ATH_PARAM_PRDPERFSTAT_PER          = 88,
#endif /* UMAC_SUPPORT_PERIODIC_PERFSTATS */
    ATH_PARAM_TOTAL_PER                = 89,
    ATH_PARAM_AMPDU_RX_BSIZE            =90,
#if ATH_SUPPORT_RX_PROC_QUOTA   
    ATH_PARAM_CNTRX_NUM                =91,
#endif   
   ATH_PARAM_RTS_CTS_RATE              =92,
#if ATH_SUPPORT_VOW_DCS
    ATH_PARAM_DCS_COCH                  =93,
    ATH_PARAM_DCS_TXERR                 =94,
    ATH_PARAM_DCS_PHYERR                =95,
#endif
#if UMAC_SUPPORT_SMARTANTENNA
    ATH_PARAM_SMARTANT_TRAIN_MODE   = 96,    /* smart antenna training mode implicit or explicit*/
    ATH_PARAM_SMARTANT_TRAIN_TYPE   = 97,    /* smart antenna train type frame base or protocol based */
    ATH_PARAM_SMARTANT_PKT_LEN      = 98,    /* packet length of the training packet */
    ATH_PARAM_SMARTANT_NUM_PKTS     = 99,    /* number of packets need to send for training */
    ATH_PARAM_SMARTANT_TRAIN_START  = 100,    /* start smart antenna training */
    ATH_PARAM_SMARTANT_NUM_ITR      = 101,    /* number of iterations needed for training */
    ATH_PARAM_SMARTANT_CURRENT_ANTENNA = 102, /* current smart antenna used for TX */
    ATH_PARAM_SMARTANT_DEFAULT_ANTENNA = 103, /* default antenna for RX */
    ATH_PARAM_SMARTANT_TRAFFIC_GEN_TIMER = 104,      /* Self packet generation timer value configuration */
    ATH_PARAM_SMARTANT_RETRAIN = 105,         /* Smart antenna retrain enable/disable */
    ATH_PARAM_SMARTANT_RETRAIN_THRESHOLD = 106,    /* number of packets required for retrain check */
    ATH_PARAM_SMARTANT_RETRAIN_INTERVAL = 107,    /* periodic retrain interval */
    ATH_PARAM_SMARTANT_RETRAIN_DROP = 108,    /* % change in goodput to tigger performance training */
    ATH_PARAM_SMARTANT_MIN_GOODPUT_THRESHOLD = 109, /* Minimum Good put threshold to tigger performance training */
    ATH_PARAM_SMARTANT_GOODPUT_AVG_INTERVAL =  110, /* Number of intervals Good put need to be averaged to use in performance training tigger */
#endif
#if ATH_RX_LOOPLIMIT_TIMER
    ATH_PARAM_LOOPLIMIT_NUM                 =  111,
#endif
#if ATH_TX_DUTY_CYCLE
    ATH_PARAM_TX_DUTY_CYCLE            = 112,
#endif
    ATH_PARAM_NODEBUG              = 113,
    ATH_PARAM_GET_IF_ID			= 114,
    ATH_PARAM_ALLOW_PROMISC = 115,
    ATH_PARAM_ANTENNA_GAIN_2G           =116,
    ATH_PARAM_ANTENNA_GAIN_5G           =117,
    ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN = 118,  /*Enable Acs back Ground Channel selection Scan timer in AP mode*/	
    ATH_PARAM_ACS_SCANTIME = 119,  /* ACS scan timer value in Seconds */
    ATH_PARAM_ACS_RSSIVAR = 120,   /*Negligence Delta RSSI between two channel */
    ATH_PARAM_ACS_CHLOADVAR = 121, /*Negligence Delta Channel load between two channel*/
    ATH_PARAM_ACS_LIMITEDOBSS = 122, /* Enable Limited OBSS check */
    ATH_PARAM_ACS_CTRLFLAG   = 123, /* Acs control flag for Scan timer */
    ATH_PARAM_ACS_DEBUGTRACE = 124, /* Acs Run time Debug level*/
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
    ATH_PARAM_BUFF_THRESH               =125,
#endif
    ATH_PARAM_BLK_REPORT_FLOOD          =126,
    ATH_PARAM_DROP_STA_QUERY            =127,
#if ATH_SUPPORT_DSCP_OVERRIDE
    ATH_PARAM_DSCP_OVERRIDE             =128,
    ATH_PARAM_DSCP_TID_MAP_RESET        =129,
    ATH_PARAM_IGMP_DSCP_OVERRIDE        =130,
    ATH_PARAM_IGMP_DSCP_TID_MAP         =131,
    ATH_PARAM_HMMC_DSCP_OVERRIDE        =132,
    ATH_PARAM_HMMC_DSCP_TID_MAP         =133,
    ATH_PARAM_UPNP_DSCP_OVERRIDE        =134,
    ATH_PARAM_UPNP_DSCP_TID_MAP         =135,
#endif
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
	ATH_PARAM_ALDSTATS                  = 136,
#endif
    ATH_PARAM_SET_FW_HANG_ID            = 137,
    ATH_PARAM_RADIO_TYPE                = 138,
    ATH_PARAM_FW_RECOVERY_ID		    = 139,
    ATH_PARAM_RATE_ENABLE_RTS           = 140,
    ATH_PARAM_EN_SELECTIVE_RTS          = 141,
    ATH_PARAM_ATH_RXQ_INFO              = 142,

    /*lifei add 2015.3 */
    QMOS_PARAM_TXQ_INFO                 = 143,
    QMOS_PARAM_WIFI_DEBUG_INFO          = 144,
    QMOS_PARAM_DROP_BCAST_PROBE         = 145,
    QMOS_PARAM_BCAST_RATE               = 146,  
    QMOS_PARAM_RADIO_INFO               = 147,
    QMOS_PARAM_STA_RATE_LIMIT           = 148,
    
} ath_param_ID_t;





//对应驱动中的国家码
#define COUNTRY_CHINA           156
#define COUNTRY_UNITED_STATES   840
#define COUNTRY_JAPAN           392
#define COUNTRY_KOREA_ROC       410
#define COUNTRY_MALAYSIA        458
#define COUNTRY_INDIA           356
#define COUNTRY_THAILAND        764
#define COUNTRY_VIET_NAM        704
#define COUNTRY_INDONESIA       360
#define COUNTRY_UNITED_KINGDOM  826
#define COUNTRY_SINGAPORE       702


enum
{
    SPECIAL_PARAM_COUNTRY_ID,/*设置国家码的子命令号*/ 
    SPECIAL_PARAM_ASF_AMEM_PRINT,
    SPECIAL_PARAM_DISP_TPC,
    SPECIAL_PARAM_ENABLE_CH_144,
    SPECIAL_PARAM_REGDOMAIN,
};


//vap扩展子命令
enum ugw_vap_cmd
{
    /*vap_cmd子集*/
	UGW_VAP_CMD_IOCTL_TEST		    = 0,	/*调试接口*/
    UGW_VAP_CMD_CHAN_SCAN	 	    = 1,	/*指定跳转信道的跨信道扫描IOCTL(扫描模式、信道列表、单信道扫描时间)*/
    UGW_VAP_CMD_GET_ASSOC_STA 	    = 2,	/*获取已关联STA的信息，包括STA的重传率、误码率*/
	UGW_VAP_CMD_GET_FLOW_STATE	    = 3,	/*获取吞吐率状态信息，radio级别，包括vap和sta吞吐率*/
	UGW_VAP_CMD_GET_CHAN_QUALITY	= 4,	/*获取工作信道的信道质量(信道号、信道利用率、误码率、噪声、重传率)*/
	UGW_VAP_CMD_GET_BSSID           = 5,    /*获取vap 的 bssid*/
    UGW_VAP_CMD_GET_CURCHAN         = 6,    /*获取当前信道*/
    UGW_VAP_CMD_STOP_SCAN           = 7,    /*停止可能存在的扫描*/
    UGW_VAP_CMD_SET_VAP_INFO        = 8,    /*设置vap的信息，当前仅包含wlan id */
    UGW_VAP_CMD_GET_MODE            = 9,    /*获取当前无线协议*/
    UGW_VAP_CMD_ADD_ACL_MAC         = 10,   /*添加黑名单MAC*/
};


//ioctl扩展子命令
enum ugw_radio_cmd
{
    UGW_RADIO_CMD_SET_RADIO_INFO        = 0,        //设置radio信息(apid,rftype)
    UGW_RADIO_CMD_SET_USERS_LIMIT       = 1,        //设置radio最大用户数
    UGW_RADIO_CMD_GET_USERS_LIMIT       = 2,        //获取radio最大用户数限制
    UGW_RADIO_CMD_GET_COUNTRY_ID        = 3,        //获取国家码
    UGW_RADIO_CMD_SET_ASSOC_STA_CYCLE   = 4,        //设置关联终端上报周期[秒] 
    UGW_RADIO_CMD_SET_RADIO_CYCLE       = 5,        //设置radio状态信息上报周期[秒]
    UGW_RADIO_CMD_SET_RADIO_MAC         = 6,        //设置radio MAC,用来使BSSID不同
    UGW_RADIO_CMD_SET_SCAN_STA_CYCLE    = 7,        //设置扫描终端上报周期[秒]
    UGW_RADIO_CMD_ADD_ACL_MAC           = 8,        //设置radio级别的黑名单
};


enum ieee80211_opmode 
{
    IEEE80211_M_STA         = 1,                 /* infrastructure station */
    IEEE80211_M_IBSS        = 0,                 /* IBSS (adhoc) station */
    IEEE80211_M_AHDEMO      = 3,                 /* Old lucent compatible adhoc demo */
    IEEE80211_M_HOSTAP      = 6,                 /* Software Access Point */
    IEEE80211_M_MONITOR     = 8,                 /* Monitor mode */
    IEEE80211_M_WDS         = 2,                 /* WDS link */
    IEEE80211_M_BTAMP       = 9,                 /* VAP for BT AMP */

    IEEE80211_M_P2P_GO      = 33,                /* P2P GO */
    IEEE80211_M_P2P_CLIENT  = 34,                /* P2P Client */
    IEEE80211_M_P2P_DEVICE  = 35,                /* P2P Device */


    IEEE80211_OPMODE_MAX    = IEEE80211_M_BTAMP, /* Highest numbered opmode in the list */

    IEEE80211_M_ANY         = 0xFF               /* Any of the above; used by NDIS 6.x */
};


struct ieee80211_clone_params
{
	char		icp_name[IFNAMSIZ];	/* device name */
	uint16_t	icp_opmode;		    /* operating mode */
	uint16_t	icp_flags;		    /* see below */
    uint8_t    icp_bssid[IEEE80211_ADDR_LEN];    /* optional mac/bssid address */
    uint8_t    icp_mataddr[IEEE80211_ADDR_LEN];  /* optional MAT address */
};


typedef struct ifdata_tlv_
{
    uint32_t    cmd;
    uint32_t    len;
    uint8_t     buf[IFDATA_TLV_BUF_LEN];
    uint8_t     buf_pad[0];
}ifdata_tlv_t;


typedef enum sfvap_mode_
{
    SFVAP_MODE_AP = 0,    /* 作为ap */
    SFVAP_MODE_STA = 1,   /* 作为sta */
    SFVAP_MODE_MAX,
}sfvap_mode_t;


#define MAX_SCAN_CHAN_CNT        24
#define IEEE80211_SCAN_PASSIVE   0x0001 /* passively scan all the channels */
#define IEEE80211_SCAN_ACTIVE    0x0002 /* actively  scan all the channels (regdomain rules still apply) */

typedef struct ugw_ext_scan_params_
{
    uint32_t    scan_mode;                             /*扫描模式(主动/被动)*/
    uint32_t    scan_channel_list[MAX_SCAN_CHAN_CNT];  /*扫描信道列表(信道序号)*/
    uint32_t    channel_num;                           /*信道个数*/
    uint32_t    scan_per_channel_time;                 /*单信道扫描时间*/

}ugw_ext_scan_params_t;

typedef struct ugw_ext_vap_info_
{
    uint32_t wlan_id;
}ugw_ext_vap_info_t;

typedef struct ugw_ext_radio_info_
{
    uint64_t    ap_devid; //APID
    uint32_t    rf_type;  //射频类型[0表示2G,1表示5G]
}ugw_ext_radio_info_t;


typedef struct ugw_sta_acl_mac_
{
    uint8_t    mac[IEEE80211_ADDR_LEN];    //添加到黑名单的MAC地址
    uint32_t   seconds;                    //黑名单有效时间 0表示永久生效
    
}ugw_sta_acl_mac_t;

// vap/radio 黑名单命令
enum
{
    SFEXT_ACL_POLICY_OPEN             = 0,/* open, don't check ACL's */
    SFEXT_ACL_POLICY_ALLOW            = 1,/* allow traffic from MAC */
    SFEXT_ACL_POLICY_DENY             = 2,/* deny traffic from MAC */
};


typedef enum
{
    DOWN = 0,
    UP = 1,
}ugw_state;

typedef enum
{
    FALSE = 0,
    TRUE = 1,
}ugw_bool;



//设置信道
int set_channel(int sock, const char *if_name, int channel_id);
//获取当前工作信道
int get_curr_channel(int sock, const char *if_name, int *curr_chan);

//设置功率
int set_power(int sock, const char *if_name, int power_val);
//获取当前功率
int get_curr_power(int sock, const char *if_name, int *curr_power);

//设置无线协议
int set_mode(int sock, const char *if_name, const char *mode, int pure); //pure表示纯协议,如果是11n映射为11nght40,需要置pure为1
int get_mode(int sock, const char *if_name, char *mode, int len);

//vap操作
int create_vap(int sock, const char *if_name, const char *vap_name, int vap_mode);
int delete_vap(int sock, const char *if_name);
int up_vap(int sock, const char *if_name);
int down_vap(int sock, const char *if_name);
int vap_set_ssid(int sock, const char *if_name, const char *essid);
int vap_set_ssid_hide(int sock, const char *if_name, int ssid_hide);

//设置VAP信息
int set_vap_info(int sock, const char *if_name, ugw_ext_vap_info_t* vap_info);

//使用SFEXT_ACL_POLICY_DENY,将VAP设置成黑名单模式
int vap_set_acl_mode(int sock, const char *if_name, int mode);

int vap_add_acl_mac(int sock, const char *if_name, ugw_sta_acl_mac_t*acl_info);

//判断接口是否存在
int interface_is_exist(int sock, const char *if_name, int *exist);//0表示不存在,1表示存在

//获取接口的up/down状态
int get_interface_state(int sock, const char *if_name, int *state);//0表示down,1表示up


//wifi操作
int down_wifi(int sock, const char *if_name);
int up_wifi(int sock, const char *if_name);
int get_wifi_state(int sock, const char *if_name, int *up);
int wifi_is_exist(int sock, const char *if_name, int *exist);

//设置radio信息
int set_radio_info(int sock, const char *if_name, ugw_ext_radio_info_t* radio_info);

//设置radio最大支持用户数
int set_radio_users_limit(int sock, const char *if_name, int max_users);
int get_radio_users_limit(int sock, const char *if_name, int *max_users);

//设置关联终端上报周期[秒] 0会取默认值
int set_assoc_sta_report_cycle(int sock, const char *if_name, int cycle);
//设置radio状态信息上报周期[秒] 0会取默认值
int set_radio_info_report_cycle(int sock, const char *if_name, int cycle);
//设置radio扫描终端上报周期[秒] 0表示不上报
int set_radio_scan_sta_cycle(int sock, const char *if_name, int cycle);

int set_radio_macaddr(int sock, const char *if_name, char *macaddr);

//国家码
int set_countrycode(int sock, const char *if_name, int country_code);

int get_countrycode(int sock, const char *if_name, int *country_code);


int get_countrycode(int sock, const char *if_name, int *country_code);

//扫描操作
int start_scan(int sock, const char *if_name, ugw_ext_scan_params_t *scan_params);
int stop_scan(int sock, const char *if_name);

//设置加密,还有问题不能用
int set_encrypt_mode(int sock, const char *if_name, const char *mode, char *password);

//桥接口操作
int br_add_if(int sock, const char *br_name, const char *if_name);
int br_del_if(int sock, const char *br_name, const char *if_name);


#pragma pack(pop)

#endif //~AP_IOCTL__H

