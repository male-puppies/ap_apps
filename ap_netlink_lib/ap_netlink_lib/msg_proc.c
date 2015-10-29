#include "ap_netlink.h"
#include <stdlib.h>


int proc_nwlan_info(ugw_netlink_data_t *report_data, int recv_len)
{
    int data_len = sizeof(ugw_netlink_data_t) + \
        report_data->count * sizeof(ugw_nwlan_info_t);
    if (recv_len != data_len)
    {
        printf("error: recv_len[%d] != data_len[%d]\n", recv_len, data_len);
        return -1;
    }

    if (NULL == report_data->buf)
    {
        printf("report_data->buf is null.\n");
        return -1;
    }
    
    ugw_nwlan_info_t *nwlans = (ugw_nwlan_info_t *)report_data->buf;

	int i;
	for (i = 0; i < report_data->count; i++)
	{
        printf("%d ssid[%-16s] bssid[%-16s] apid[%-16s] wlanid[%-4u] "
                "rf_type[%-4u] channel[%-3u] rssi[%-4d]\n", \
                i, \
                nwlans[i].ssid, MAC_STR(nwlans[i].bssid), \
                PRINT_APID(nwlans[i].ap_devid), nwlans[i].wlan_id, \
                nwlans[i].rf_type, nwlans[i].channel_id, nwlans[i].rssi-100);
	}

    return 0;
}


int proc_assoc_sta_info(ugw_netlink_data_t *report_data, int recv_len)
{
    int data_len = sizeof(ugw_netlink_data_t) + \
        report_data->count * sizeof(ugw_sta_info_t);
    if (recv_len != data_len)
    {
        printf("error: recv_len[%d] != data_len[%d]\n", recv_len, data_len);
        return -1;
    }
    
    if (NULL == report_data->buf)
    {
        printf("report_data->buf is null.\n");
        return -1;
    }

    ugw_sta_info_t *assoc_stas = (ugw_sta_info_t *)report_data->buf;

    int i;
    for (i = 0; i < report_data->count; i++)
    {
        printf("mac[%-16s] ip[%u.%u.%u.%u] ssid[%-16s] ", \
            MAC_STR(assoc_stas[i].sta_mac), \
            assoc_stas[i].ip[0], assoc_stas[i].ip[1], \
            assoc_stas[i].ip[2], assoc_stas[i].ip[3], \
            assoc_stas[i].ssid);
        
        printf("bssid[%-16s] rssi[%-4d] "
            "rf_type[%u] dual_band[%u] channel[%-3u] "
            "avg_tx_goodput:%uB/s avg_rx_goodput:%uB/s\n", \
            MAC_STR(assoc_stas[i].bssid), (assoc_stas[i].rssi-100), \
            assoc_stas[i].rf_type, assoc_stas[i].is_dualband, \
            assoc_stas[i].channel_id, \
            assoc_stas[i].avg_tx_bytes, \
            assoc_stas[i].avg_rx_bytes);
    }
    
    return 0;
}



int proc_radio_info(ugw_netlink_data_t *report_data, int recv_len)
{
    int data_len = sizeof(ugw_netlink_data_t) + \
        report_data->count * sizeof(ugw_radio_info_t);
    if (recv_len != data_len)
    {
        printf("error: recv_len[%d] != data_len[%d]\n", recv_len, data_len);
        return -1;
    }
    
    if (NULL == report_data->buf)
    {
        printf("report_data->buf is null.\n");
        return -1;
    }

    ugw_radio_info_t *radio_info = (ugw_radio_info_t *)report_data->buf;
    printf("mode[%s] rf_type[%u] channel[%u] txpower[%u] "
        "users[%u] channel_use[%u] noise[%d]\n", \
        radio_info->mode, radio_info->rf_type, radio_info->channel_id, \
        radio_info->txpower, radio_info->curr_users, \
        radio_info->channel_use, radio_info->noise);

    return 0;
}

int proc_sta_auth_notif(ugw_netlink_data_t *report_data, int recv_len)
{
    int data_len = sizeof(ugw_netlink_data_t) + \
        report_data->count * sizeof(ugw_sta_auth_notif_t);
    if (recv_len != data_len)
    {
        printf("error: recv_len[%d] != data_len[%d]\n", recv_len, data_len);
        return -1;
    }

    if (NULL == report_data->buf)
    {
        printf("report_data->buf is null.\n");
        return -1;
    }

    ugw_sta_auth_notif_t *sta_notif = (ugw_sta_auth_notif_t *)report_data->buf;
    printf("mac[%-16s] ap_devid[%016"PRIX64"] rf_type[%u] curr_users[%u] ", \
        MAC_STR(sta_notif->sta_mac), sta_notif->ap_devid, sta_notif->rf_type, \
        sta_notif->curr_users);
    printf("bssid[%-16s] wlan_id[%u] action[%u]\n", \
        MAC_STR(sta_notif->bssid), sta_notif->wlan_id, sta_notif->action);

    return 0;
}


int proc_scan_sta_info(ugw_netlink_data_t *report_data, int recv_len)
{
    int data_len = sizeof(ugw_netlink_data_t) + \
        report_data->count * sizeof(ugw_scan_sta_info_t);
    if (recv_len != data_len)
    {
        printf("error: recv_len[%d] != data_len[%d] ugw_scan_sta_info_t:%d\n", \
            recv_len, data_len, sizeof(ugw_scan_sta_info_t));
        return -1;
    }

    if (NULL == report_data->buf)
    {
        printf("report_data->buf is null.\n");
        return -1;
    }

    ugw_scan_sta_info_t *scan_stas = (ugw_scan_sta_info_t *)report_data->buf;
    int i = 0;
    for (; i < report_data->count; i++)
    {
        printf("[%-2d]mac[%-16s] rssi[%d]dBm rf_type[%d]\n", \
            i, \
            MAC_STR(scan_stas[i].sta_mac), \
            scan_stas[i].rssi-100, \
            scan_stas[i].rf_type);
    }

    return 0;
}


int msg_proc(ugw_netlink_data_t *report_data, int recv_len, int print_mask)
{
    int error = 0;
    
    if (NULL == report_data)
    {
        return -1;
    }
   
	if ((print_mask & (1<<(report_data->type))) == 0)
	{
		return 0;
	}
   	
   	printf("msg type:%u subtype:%u count:%u data_len:%d\n", \
        		report_data->type, report_data->subtype, report_data->count, recv_len);
    
    switch(report_data->type)
    {
        case UGW_NEIGHBOR_WLAN_INFO:
            error = proc_nwlan_info(report_data, recv_len);
            break;
            
        case UGW_ASSOC_STA_INFO:
            error = proc_assoc_sta_info(report_data, recv_len);
            break;

        case UGW_RADIO_INFO:
            error = proc_radio_info(report_data, recv_len);
            break;
            
        case UGW_STA_AUTH_NOTIF:
            error = proc_sta_auth_notif(report_data, recv_len);
            break;
            
        case UGW_SCAN_STA_INFO:
            error = proc_scan_sta_info(report_data, recv_len);
            break;
            
        default:
            printf("nelink type error:%d\n", report_data->type);
            break;
    }

    printf("\n");
    return error;
}


int main(int argc , char* argv[])
{	
    int print_mask = 0xffffffff;
    if (argc > 1)
    {  
    	print_mask = strtol(argv[argc-1], NULL, 16);        
    }    
    if (print_mask == 0)
        print_mask = ((1<<UGW_NEIGHBOR_WLAN_INFO) | (1<<UGW_RADIO_INFO) | (1<<UGW_STA_AUTH_NOTIF));
    printf("\n************print mask=[0x%X]\n", print_mask);  
	
	char str[] = "hello world";
    ugw_netlink_init();	
    printf("************sock_fd:%d.\n", ugw_netlink_sock);
	ugw_netlink_sendmsg(str, sizeof(str));
	
	while (1)
	{
        ugw_netlink_data_t *report_data;
        int recv_len;
		ugw_netlink_recvmsg((void**)&report_data, &recv_len);
        if (-1 == msg_proc(report_data, recv_len, print_mask))
        {
            continue;
        }

		usleep(100000);
	}
    
	ugw_netlink_destroy();
	
	return 0;
}



