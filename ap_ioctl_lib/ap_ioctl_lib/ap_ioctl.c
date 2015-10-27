#include "ap_ioctl.h"


int set80211priv(int sock, const char *if_name, int option, void *data, size_t len)
{
    if (sock < 0 || NULL==if_name || NULL==data)
	{
		printf("Invalid argument\n");
		return -1;
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
		printf("set80211priv %s option[%d] datalen:%d set fail:%s\n", \
            if_name, option, len, strerror(errno));
	}

	return err < 0 ? errno : 0;
}


int set_param(int sock, const char *if_name, int option, int flag)
{
    if (sock < 0 || NULL==if_name)
	{
		printf("Invalid argument\n");
		return -1;
	}
    
	int param[2];
	param[0] = option;
	param[1] = flag;

	return set80211priv(sock, if_name, IEEE80211_IOCTL_SETPARAM, \
		(void *)&param, sizeof(param));
}


//设置信道
int set_channel(int sock, const char *if_name, int channel_id)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
	struct iwreq iwr = { { { 0 } } };
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
	iwr.u.freq.m = channel_id;
	iwr.u.freq.e = 0;
	iwr.u.freq.flags = IW_FREQ_FIXED;

    int err = ioctl(sock, SIOCSIWFREQ, &iwr);
	if (err < 0)
	{
		printf("set %s channel %u fail:%s\n", \
            if_name, channel_id, strerror(errno));
	}

	return err < 0 ? errno : 0;
}


//获取当前工作信道
int get_curr_channel(int sock, const char *if_name, int *curr_chan)
{
    if (sock < 0 || NULL==if_name || NULL==curr_chan)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct iwreq iwr;
    ifdata_tlv_t tlv;
    
    tlv.cmd = UGW_VAP_CMD_GET_CURCHAN;
    tlv.len = sizeof(unsigned int);
    
    memset(&iwr, 0, sizeof(iwr));
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv) + 1;
    
    strncpy(iwr.ifr_name, if_name, IFNAMSIZ);
    iwr.ifr_name[IFNAMSIZ - 1] = '\0';

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("Get %s cur_chan error:%s\n", if_name, strerror(errno));
    }
    else
    {
        memcpy(curr_chan, &(tlv.buf), tlv.len);
    }

	return err < 0 ? errno : 0;
}



//设置功率
int set_power(int sock, const char *if_name, int power_val)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
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
		printf("set %s Tx txpower %d failed:%s\n", if_name, power_val, strerror(errno));
	}

	return err < 0 ? errno : 0;
}

//获取vap当前功率
int get_curr_power(int sock, const char *if_name, int *curr_power)
{
    if (sock < 0 || NULL==if_name || NULL==curr_power)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct iwreq iwr = { { { 0 } } };
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    int err = ioctl(sock, SIOCGIWTXPOW, &iwr);
    if (err < 0)
    {
        printf("Get curr_TxPower on %s failed:%s\n", if_name, strerror(errno));
    }
    else
    {
        *curr_power = iwr.u.txpower.value;
    }
    
	return err < 0 ? errno : 0;
}

void strupper(char *str)
{
	if (NULL == str)
	{
		return ;
	}
	
	int str_len = strlen(str);
	int i;
	for (i = 0; i < str_len; i++)
	{
		if (!isupper(str[i]))
		{
			str[i] = toupper(str[i]);
		}
	}
	
	return ;
}


#define WIFI_NAME_0 "wifi0"
#define WIFI_NAME_1 "wifi1"
const char *qmosar_get_wifi_name(const char *ifname)
{
    if (!strncmp(ifname, "ath2g_", sizeof("ath2g_") - 1))
    {
        return WIFI_NAME_0;
    }
    return WIFI_NAME_1;    
}


static int qmosar_set_a_msdu_enable(int sock, const char *ifname, unsigned char enable)
{
    int err = 0;
    int param[2];
    param[0] = ATH_PARAM_AMSDU_ENABLE | ATH_PARAM_SHIFT; 
    param[1] = enable;

    if (set80211priv(sock, qmosar_get_wifi_name(ifname), SIOCIWFIRSTPRIV,
            &param, sizeof(param)) < 0) 
    {
        printf("%s: set A-MSDU %d for %s failed:%s.\n", __func__, enable, ifname, strerror(errno));
        err = -1;
    }    

    printf("%s(): result = %d. para[ifname=%s, enable=%d wifi=%s] \n",
            __func__, err, ifname, enable, qmosar_get_wifi_name(ifname));
    return err;
}



int qmosar_set_a_mpdu_enable(int sock, const char *ifname, unsigned char a_mpdu_enable)
{    
    int param[2];
    printf("%s. set A-MPDU %d for %s start", __func__, a_mpdu_enable, ifname);

    param[0] = ATH_PARAM_AMPDU | ATH_PARAM_SHIFT; 
    param[1] = a_mpdu_enable;        
    if (set80211priv(sock, qmosar_get_wifi_name(ifname), SIOCIWFIRSTPRIV, &param, sizeof(param)) < 0) 
    {
        printf("%s set A-MPDU %d for %s failed:%s", __func__, a_mpdu_enable,
               ifname, strerror(errno));
        return -1;
    }   

    param[0] = IEEE80211_PARAM_AMPDU;
    param[1] = a_mpdu_enable;    
    if (set80211priv(sock, ifname, IEEE80211_IOCTL_SETPARAM, &param, sizeof(param)) < 0)
    {
        printf("Set A-MPDU %d for %s failed:%s", a_mpdu_enable, ifname, strerror(errno));
        return -1;        
    }
    
    return 0;
}


/*
PREAM_SHORT:
  short_pream = 1;
PREAM_LONG:
  short_pream = 0;
*/
static int qmosar_set_pream_type(int sock, const char *ifname, int short_pream)
{
    int err = 0;
    int param[2];       
    param[0] = IEEE80211_PARAM_SHORTPREAMBLE;
    param[1] = short_pream;
    if (set80211priv(sock, ifname, IEEE80211_IOCTL_SETPARAM,
            &param, sizeof(param)) < 0) 
    {
        printf("%s: set pream_type %d for %s failed:%s \n", __func__, short_pream, ifname, strerror(errno));
        err = -1;
    }
    
    printf("%s(): result = %d. para[ifname=%s, short_pream=%d] \n", __func__, err, ifname, short_pream);
    return err;
}


static int qmosar_set_short_gi(int sock, const char *ifname, unsigned char short_gi)
{    
    int err = 0;
    int param[2];        

    param[0] = IEEE80211_PARAM_SHORT_GI;
    param[1] = short_gi;
    if (set80211priv(sock, ifname, IEEE80211_IOCTL_SETPARAM, &param, sizeof(param)) < 0) 
    {
        printf("set short_gi %d for %s failed:%s  \n", short_gi, ifname, strerror(errno));
        err =  -1;
    }
    
    printf("%s(): result = %d. para[ifname=%s, short_gi=%d]  \n", __func__, err, ifname, short_gi);
    return 0;
}


static void qmosar_set_pure(int sock, const char *ifname, char *mode, int pure)
{   
    int err = 0;
    set_param(sock, ifname, IEEE80211_PARAM_PUREG, 0);
    set_param(sock, ifname, IEEE80211_PARAM_PUREN, 0);
    
	if (pure)
	{
		if (!strcmp(mode, "11G"))
		{
			err = set_param(sock, ifname, IEEE80211_PARAM_PUREG, 1);
		}
		else if (!strcmp(mode, "11NGHT20") || !strcmp(mode, "11NGHT40") || 
			!strcmp(mode, "11NAHT20") || !strcmp(mode, "11NAHT40"))
		{
            err = set_param(sock, ifname, IEEE80211_PARAM_PUREN, 1);
		}		
	}    

    printf("%s(): result = %d. para[ifname=%s, pure=%d] \n", __func__, err, ifname, pure);
}


static void qmosar_set_cw(int sock, const char *ifname, char *mode)
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
    
    int err = set_param(sock, ifname, IEEE80211_PARAM_CWM_ENABLE, cwmenable);    
    int err2 = set_param(sock, ifname, IEEE80211_PARAM_COEXT_DISABLE, disablecoext);     
    printf("%s(): result = %d,%d. para[ifname=%s, mode=%s] \n", __func__, err, err2, ifname, mode);
}



//设置无线协议
int set_mode(int sock, const char *if_name, const char *mode_str, int pure)
{
    if (sock < 0 || NULL==if_name || NULL==mode_str)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
#define MODE_LEN 50
	char mode[MODE_LEN + 1] = {0};
	int len = strlen(mode_str);
	if (len > MODE_LEN)
		len = MODE_LEN;
	
	strncpy(mode, mode_str, len);
	
	//全部转为大写
	strupper(mode);	
	
	struct iwreq iwr = { { {0} } };
	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);
	iwr.u.data.pointer = (void *)mode;
	iwr.u.data.length = strlen(mode) + 1;    /*+1不然容易出现Invalid argument */

    set_channel(sock, if_name, 0);
    
    int err = ioctl(sock, IEEE80211_IOCTL_SETMODE, &iwr);
	if (err < 0)
	{
		printf("%s set %s rf mode %s fail:%s\n", __func__, if_name, mode, strerror(errno));
		goto ERR;
	}

    
    qmosar_set_pure(sock, if_name, mode, pure);
    qmosar_set_cw(sock, if_name, mode);
    qmosar_set_a_mpdu_enable(sock, if_name, 1);
    qmosar_set_a_msdu_enable(sock, if_name, 1);
    qmosar_set_pream_type(sock, if_name, 0);
    qmosar_set_short_gi(sock, if_name, 0);
ERR:
	return err < 0 ? errno : 0;
}



int get_mode(int sock, const char *if_name, char *mode, int len)
{
    if (sock < 0 || NULL==if_name || NULL==mode)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct iwreq iwr;
    ifdata_tlv_t tlv;
    
    tlv.cmd = UGW_VAP_CMD_GET_MODE;
    tlv.len = len;
    
    memset(&iwr, 0, sizeof(iwr));
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv) + 1;
    
    strncpy(iwr.ifr_name, if_name, IFNAMSIZ);
    iwr.ifr_name[IFNAMSIZ - 1] = '\0';

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("Get %s current mode error:%s\n", if_name, strerror(errno));
    }
    else
    {
        if (tlv.len > len)
            tlv.len = len;
        
        memcpy(mode, &(tlv.buf), tlv.len);
    }
    
    return err < 0 ? errno : 0;
}


//创建vap
int create_vap(int sock, const char *if_name, const char *vap_name, int vap_mode)
{
    if (sock < 0 || NULL==if_name || NULL==vap_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    
    struct ifreq ifr;
    struct ieee80211_clone_params cp;
    
    memset(&ifr, 0, sizeof(ifr));
    memset(&cp, 0, sizeof(cp));

    /* ath x/x */
    strncpy(cp.icp_name, vap_name, IFNAMSIZ);    
    /* wifi x */
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    
    switch(vap_mode)
    {
        case SFVAP_MODE_AP:
            cp.icp_opmode = IEEE80211_M_HOSTAP;
            break;
        case SFVAP_MODE_STA:
            cp.icp_opmode = IEEE80211_M_STA;
            break;
        default:
            printf("%s create vap[%s] error, wrong vap_mode %d\n", \
                if_name, vap_name, vap_mode);
            return -1;
    }

    cp.icp_opmode = IEEE80211_M_HOSTAP;
    cp.icp_flags = IEEE80211_CLONE_BSSID;
    
    ifr.ifr_data  = (void *) &cp;

    int err = ioctl(sock, SIOC80211IFCREATE, &ifr);
    if (err < 0)
    {
        printf("%s create vap[%s] failed:%s\n", \
            if_name, vap_name, strerror(errno));
    }
    
    return err < 0 ? errno : 0;
}



//删除vap
int delete_vap(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    int err = ioctl(sock, SIOC80211IFDESTROY, &ifr);
    if (err < 0)
    {
        printf("delete vap %s failed:%s\n", if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}



int down_vap(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
	
	struct ifreq ifr;
    int err = 0;
	
#if 0
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));

    err = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (err < 0) 
    {    
        printf("ioctl[SIOCGIFFLAGS] %s failed: %s\n", \
        	if_name, strerror(errno));
        
        return errno;
    }

    if (!(ifr.ifr_flags & IFF_UP)) 
    {
        printf("Interface %s is already down\n", if_name);

        return 0;
    }

    ifr.ifr_flags &= ~IFF_UP;
    
    err = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (err < 0)
    {
        printf("ioctl[SIOCSIFFLAGS] %s failed: %s\n", \
        	if_name, strerror(errno));
    }
#endif

	char cmd[CMD_LEN] = {0};
	snprintf(cmd, CMD_LEN, "ifconfig %s down", if_name);
	system(cmd);
	
	return err < 0 ? errno : 0;
}



int up_vap(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

	struct ifreq ifr;
    int err = 0;

#if 0
	memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));

    err = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (err < 0) 
    {    
        printf("ioctl[SIOCGIFFLAGS] %s failed: %s\n", \
        	if_name, strerror(errno)); 
            
        return errno;
    }

    if (ifr.ifr_flags & IFF_UP) 
    {
        printf("Interface %s is already up\n", if_name);
		down_vap(sock, if_name);
        //return 0;
    }
	
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    err = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (err < 0)
    {
        printf("ioctl[SIOCSIFFLAGS] %s failed: %s\n", \
        	if_name, strerror(errno));
    }
#endif

	char cmd[CMD_LEN] = {0};
	snprintf(cmd, CMD_LEN, "ifconfig %s up", if_name);
	system(cmd);
	
	return err < 0 ? errno : 0;
}



//设置vap的SSID名称
int vap_set_ssid(int sock, const char *if_name, const char *essid)
{
    if (sock < 0 || NULL==if_name || NULL==essid)
    {
        printf("Invalid argument\n");
        return -1;
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
        printf("%s Set SSID %s failed:%s\n", \
            if_name, essid, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


//设置SSID隐藏
int vap_set_ssid_hide(int sock, const char *if_name, int ssid_hide)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    return set_param(sock, if_name, IEEE80211_PARAM_HIDESSID, ssid_hide);    
}


//设置VAP信息
int set_vap_info(int sock, const char *if_name, ugw_ext_vap_info_t* vap_info)
{
    if (sock < 0 || NULL==if_name || NULL==vap_info)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    ifdata_tlv_t        tlv;
    struct iwreq        iwr         = { { { 0 } } };

    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    tlv.cmd = UGW_VAP_CMD_SET_VAP_INFO;
    tlv.len = sizeof(ugw_ext_vap_info_t);

    memcpy(tlv.buf, (void *)vap_info, tlv.len);
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv);

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("%s Set vap_info failed:%s\n", if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


int vap_set_acl_mode(int sock, const char *if_name, int mode)
{
    if (sock < 0 || NULL == if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    return set_param(sock, if_name, IEEE80211_PARAM_MACCMD, mode);
}

int vap_add_acl_mac(int sock, const char *if_name, ugw_sta_acl_mac_t *acl_info)
{
    if (sock < 0 || NULL == if_name || NULL==acl_info)
    {
        printf("Invalid argument\n");
        return -1;
    }

    ifdata_tlv_t        tlv;
    struct iwreq        iwr         = { { { 0 } } };
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    tlv.cmd = UGW_VAP_CMD_ADD_ACL_MAC;
    tlv.len = sizeof(ugw_sta_acl_mac_t);

    memcpy(tlv.buf, (void *)acl_info, tlv.len);
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv);

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("%s Set vap acl mac failed:%s\n", if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


int radio_add_acl_mac(int sock, const char *if_name, ugw_sta_acl_mac_t *acl_info)
{
    if (sock < 0 || NULL == if_name || NULL==acl_info)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_ADD_ACL_MAC;
    tlv.len = sizeof(ugw_sta_acl_mac_t);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    memcpy(&(tlv.buf), (void *)acl_info, tlv.len);
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Set %s radio acl mac failed:%s\n", \
            if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


int interface_is_exist(int sock, const char *if_name, int *exist)
{
    if (sock < 0 || NULL==if_name || NULL==exist)
    {
        printf("Invalid argument\n");
        return -1;
    }

    *exist = TRUE;
    
    struct ifreq ifr;
    int err = 0;
    unsigned int if_index = if_nametoindex(if_name);
    if (0 == if_index)
    {
        *exist = FALSE;
    }

    return 0;
}


int get_interface_state(int sock, const char *if_name, int *state)
{
    if (sock < 0 || NULL==if_name || NULL==state)
    {
        printf("Invalid argument\n");
        return -1;
    }

 	struct ifreq ifr;
    int err = 0;
    
	memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));

    err = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (err < 0) 
    {    
        printf("ioctl[SIOCGIFFLAGS] %s failed: %s\n", \
        	if_name, strerror(errno)); 
            
        return errno;
    }

    if (ifr.ifr_flags & IFF_UP) 
    {
        *state = UP;
    }
    else
    {
        *state = DOWN;
    }

    return 0;
}


int down_wifi(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct ifreq ifr;
    int err = 0;
 
#if 0 
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    err = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (err < 0) 
    {    
        printf("ioctl[SIOCGIFFLAGS] %s failed: %s\n", \
            if_name, strerror(errno));
        return errno;
    }
    
    if (!(ifr.ifr_flags & IFF_UP)) 
    {
        printf("Interface %s is already down\n", if_name);

        return 0;
    }

    ifr.ifr_flags &= ~IFF_UP;

    err = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (err < 0) 
    {
        printf("ioctl[SIOCSIFFLAGS] %s failed: %s\n", \
            if_name, strerror(errno));
    }
#endif

	char cmd[CMD_LEN] = {0};
	snprintf(cmd, CMD_LEN, "ifconfig %s down", if_name);
	system(cmd);
	
    return err < 0 ? errno : 0;
}


int up_wifi(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
	
#if 0
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) != 0) 
    {    
        printf("ioctl[SIOCGIFFLAGS] %s failed: %s\n", \
            if_name, strerror(errno));   
               
        return -1;
    }
    
    if (ifr.ifr_flags & IFF_UP)
    {
        printf("Interface %s is already running\n", if_name);
		down_wifi(sock, if_name);
        //return 0;
    }

    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

    int err = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (err < 0)
    {
        printf("ioctl[SIOCSIFFLAGS] %s failed: %s\n", \
            if_name, strerror(errno));
    }
	
#endif

	char cmd[CMD_LEN] = {0};
	snprintf(cmd, CMD_LEN, "ifconfig %s up", if_name);
	system(cmd);
	
	return 0;
    //return err < 0 ? errno : 0;
}


//设置国家码
int set_countrycode(int sock, const char *if_name, int country_code)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    return set_param(sock, if_name, \
        SPECIAL_PARAM_SHIFT|SPECIAL_PARAM_COUNTRY_ID, country_code);
}


int get_countrycode(int sock, const char *if_name, int *country_code)
{
    if (sock < 0 || NULL==if_name || NULL==country_code)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_GET_COUNTRY_ID;
    tlv.len = sizeof(int);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Get %s country code error:%s\n", if_name, strerror(errno));
    }
    else
    {
       memcpy(country_code, &(tlv.buf), tlv.len);
    }

    return err < 0 ? errno : 0;
}



/*显示当前系统时间*/
void print_system_time(void)
{
    time_t       now;              //实例化time_t结构
    struct tm    timenow;          //实例化tm结构指针

    time(&now);                    
    localtime_r(&now, &timenow);
    
    printf("Display systime: now %d:%d:%d\n", \
        timenow.tm_hour, timenow.tm_min, timenow.tm_sec);

    return ;
}


//启动扫描
int start_scan(int sock, const char *if_name, ugw_ext_scan_params_t *scan_params)
{
    if (sock < 0 || NULL==if_name || NULL==scan_params)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct iwreq iwr = { { { 0 } } };
    ifdata_tlv_t tlv;

    //print_system_time();
    
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    tlv.cmd = UGW_VAP_CMD_CHAN_SCAN;
    tlv.len = sizeof(ugw_ext_scan_params_t);

    memcpy(&(tlv.buf), (void *)(scan_params), tlv.len);
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv);

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("%s Start Scan channel failed:%s\n", \
            if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


//停止扫描
int stop_scan(int sock, const char *if_name)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct iwreq iwr = { { { 0 } } };
    ifdata_tlv_t tlv;
    
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    tlv.cmd = UGW_VAP_CMD_STOP_SCAN;
    tlv.len = 0;
    iwr.u.data.pointer = &tlv;
    iwr.u.data.flags = 0;
    iwr.u.data.length = sizeof(tlv);

    int err = ioctl(sock, IEEE80211_IOCTL_UGW_EXT, &iwr);
    if(err < 0)
    {    
        printf("Stop Scan on %s failed:%s\n", if_name, strerror(errno));
    }
    
    return err < 0 ? errno : 0;
}


//设置加密open,wep
int set_encrypt_mode(int sock, const char *if_name, const char *mode, char *password)
{
    if (sock < 0 || NULL==if_name || NULL==mode)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct iwreq iwr = { { { 0 } } };
    snprintf(iwr.ifr_name, IFNAMSIZ, "%s", if_name);

    if (NULL != password)
    {
        iwr.u.data.pointer = password;
        iwr.u.data.length = strlen(password)+1;
    }

    if (!strcmp(mode, "open"))
        iwr.u.data.flags |= IW_ENCODE_DISABLED;
    else if (!strcmp(mode, "restricted"))
        iwr.u.data.flags |= IW_ENCODE_RESTRICTED;
    else
        printf("%s mode:%s illegal\n", if_name, mode);

    int err = ioctl(sock, SIOCSIWENCODE, &iwr);
    if(err < 0)
    {
        printf("%s set encrypt fail:%s\n", if_name, strerror(errno));
    }
    
    return err < 0 ? errno : 0;
}



//添加接口到桥
int br_add_if(int sock, const char *br_name, const char *if_name)
{
    if (sock < 0 || NULL==br_name || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct ifreq ifr;
    int err = 0;
    unsigned int if_index = if_nametoindex(if_name);

    if (0 == if_index)
    {
        printf("No such device:%s\n", if_name);
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
        printf("device:%s is already a member of a bridg:%s\n", \
            if_name, br_name);
        err = 0; //不报错
    }

    return err < 0 ? errno : 0;
}



//删除桥中的接口
int br_del_if(int sock, const char *br_name, const char *if_name)
{
    if (sock < 0 || NULL==br_name || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    int err;
    int ifindex = if_nametoindex(if_name);
    
	if (0 == ifindex)
	{
        printf("No such device:%s\n", if_name);
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
        printf("device %s is not a slave of %s\n", if_name, br_name);
        err = 0;
    }
    
    return err < 0 ? errno : 0;
}


//设置radio信息
int set_radio_info(int sock, const char *if_name, ugw_ext_radio_info_t* radio_info)
{
    if (sock < 0 || NULL==if_name || NULL==radio_info)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq        ifr;
    ifdata_tlv_t        tlv;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    tlv.cmd = UGW_RADIO_CMD_SET_RADIO_INFO;
    tlv.len = sizeof(ugw_ext_radio_info_t);
    memcpy(tlv.buf, radio_info, sizeof(ugw_ext_radio_info_t));
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {
        printf("%s Set radio_info error:%s\n", if_name, strerror(errno));
    }
    
    return err < 0 ? errno : 0;
}


//设置radio最大支持用户数
int set_radio_users_limit(int sock, const char *if_name, int max_users)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq        ifr;
    ifdata_tlv_t        tlv;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    tlv.cmd = UGW_RADIO_CMD_SET_USERS_LIMIT;
    tlv.len = sizeof(u_int32_t);
    memcpy(&(tlv.buf), (void *)(&max_users), tlv.len);
    ifr.ifr_data = (void *) &tlv;   

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
        printf("%s Set max_users[%d] failed:%s\n", \
            if_name, max_users, strerror(errno));
    }

    return err < 0 ? errno : 0;
}



int get_radio_users_limit(int sock, const char *if_name, int *users_limit)
{
    if (sock < 0 || NULL==if_name || NULL==users_limit)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_GET_USERS_LIMIT;
    tlv.len = sizeof(int);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Get %s radio users limit error:%s\n", if_name, strerror(errno));
    }
    else
    {
       memcpy(users_limit, &(tlv.buf), tlv.len);
    }

    return err < 0 ? errno : 0;
}



int set_assoc_sta_report_cycle(int sock, const char *if_name, int cycle)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_SET_ASSOC_STA_CYCLE;
    tlv.len = sizeof(int);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    memcpy(&(tlv.buf), (void *)(&cycle), tlv.len);
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Set %s radio assoc sta report cycle error:%s\n", \
            if_name, strerror(errno));
    }
    
    return err < 0 ? errno : 0;
}



int set_radio_info_report_cycle(int sock, const char *if_name, int cycle)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_SET_RADIO_CYCLE;
    tlv.len = sizeof(int);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    memcpy(&(tlv.buf), (void *)(&cycle), tlv.len);
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Set %s radio info report cycle error:%s\n", \
            if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


int set_radio_macaddr(int sock, const char *if_name, char *macaddr)
{
    if (sock < 0 || NULL==if_name || NULL==macaddr)
    {
        printf("Invalid argument\n");
        return -1;
    }

    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_SET_RADIO_MAC;
    tlv.len = IEEE80211_ADDR_LEN;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    memcpy(&(tlv.buf), (void *)macaddr, tlv.len);
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Set %s radio MAC address error:%s\n", \
            if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


int set_radio_scan_sta_cycle(int sock, const char *if_name, int cycle)
{
    if (sock < 0 || NULL==if_name)
    {
        printf("Invalid argument\n");
        return -1;
    }
    
    struct ifreq ifr;
    ifdata_tlv_t tlv;
    tlv.cmd = UGW_RADIO_CMD_SET_SCAN_STA_CYCLE;
    tlv.len = sizeof(int);

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    memcpy(&(tlv.buf), (void *)(&cycle), tlv.len);
    ifr.ifr_data = (void *)&tlv;

    int err = ioctl(sock, SIOCUGWEXT, &ifr);
    if(err < 0)
    {    
       printf("Set %s radio scan sta report cycle error:%s\n", \
            if_name, strerror(errno));
    }

    return err < 0 ? errno : 0;
}


