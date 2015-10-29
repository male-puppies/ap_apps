/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 21 Aug 2015 14:39:44 +0800
 */
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "auth_client.h"
#include "auth_misc.h"
#include "auth_mqtt.h"
#include "auth_user.h"
#include "cJSON.h"

int auth_client_set_online(const char *data)
{
	int n;
	unsigned char macaddr[6];
	unsigned int mac[6];
	char ssid[64];
	unsigned int uid;

	memset(ssid, 0, sizeof(ssid));
	n = sscanf(data, "%02X:%02X:%02X:%02X:%02X:%02X,%[^\n^,],%X\n",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
			(char *)&ssid, &uid);

	if (n != 8)
	{
		return -1;
	}

	macaddr[0] = mac[0];
	macaddr[1] = mac[1];
	macaddr[2] = mac[2];
	macaddr[3] = mac[3];
	macaddr[4] = mac[4];
	macaddr[5] = mac[5];

	return AUTH_USER_SET_ONLINE(macaddr, ssid, uid);
}

int auth_client_check_roaming(const unsigned char *mac, const char *ssid)
{
	char buf[128];
	char *msgstr;
	cJSON *msg;

	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X,%s\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ssid);
	
	msg = cJSON_CreateObject();

	if (msg)
	{
		cJSON_AddStringToObject(msg, "cmd", "check_online");
		cJSON_AddStringToObject(msg, "data", buf);

		msgstr = cJSON_PrintUnformatted(msg);
		if (msgstr)
		{
			printf("%s\n", msgstr);
			auth_mqtt_publish("a/vlan/local", msgstr, strlen(msgstr));
			free(msgstr);
		}
		cJSON_Delete(msg);
	}

	return 0;
}

int auth_client_login_publish(const unsigned char *mac, const char *ssid, unsigned int uid)
{
	char buf[128];
	char *msgstr;
	cJSON *msg;

	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X,%s,%X\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
			ssid, uid);
	
	msg = cJSON_CreateObject();
	cJSON_AddStringToObject(msg, "cmd", "login_success");
	cJSON_AddStringToObject(msg, "data", buf);

	msgstr = cJSON_PrintUnformatted(msg);
	printf("%s\n", msgstr);
	auth_mqtt_publish("a/vlan/local", msgstr, strlen(msgstr));

	cJSON_Delete(msg);
	free(msgstr);

	return 0;
}

int auth_client_check_server(time_t current_time)
{
	static time_t last_time = 0;
	char *msgstr;
	cJSON *msgJS;
	cJSON *dataJS;

	if (current_time - last_time < 300)
	{
		return 0;
	}
	last_time = current_time;

	msgJS = cJSON_CreateObject();
	cJSON_AddStringToObject(msgJS, "out_topic", "a/ac/usrmgr/query");
	cJSON_AddNumberToObject(msgJS, "deadline", current_time + 10);

	dataJS = cJSON_CreateObject();
	cJSON_AddStringToObject(dataJS, "mod", "a/local/auth");
	cJSON_AddStringToObject(dataJS, "cmd", "check_config_update");
	cJSON_AddStringToObject(dataJS, "group", auth_user_ac_account);
	cJSON_AddNumberToObject(dataJS, "config_version", auth_config_version);

	cJSON_AddItemToObject(msgJS, "data", dataJS);

	msgstr = cJSON_PrintUnformatted(msgJS);
	printf("%s\n", msgstr);
	auth_mqtt_publish("a/local/proxy", msgstr, strlen(msgstr));

	cJSON_Delete(msgJS);
	free(msgstr);

	return 0;
}

int auth_client_push_config(const char *data)
{
	printf("push_config\n%s\n", data);
	cJSON *uJS = cJSON_Parse(data);

	if (uJS)
	{
		cJSON *js = cJSON_GetObjectItem(uJS, "ConfigVersion");
		if (js)
		{
			if (js->valueint != auth_config_version)
			{
				printf("version diff %d %d\n", auth_config_version, js->valueint);
				auth_misc_io_store("/tmp/authd.json", data, strlen(data));
				system("cp /tmp/authd.json /ugwconfig/etc/ap/authd.json");
				auth_config_version = js->valueint;
				update_on_connect_msg();
			}
		}

		auth_ctrl_config_reload(uJS);
		auth_users_reload(uJS);
		cJSON_Delete(uJS);
	}

	return 0;
}
