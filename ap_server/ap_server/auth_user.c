/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 24 Jul 2015 15:15:01 +0800
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "list.h"
#include "auth_user.h"
#include "auth_misc.h"
#include "cJSON.h"

char auth_user_ac_account[64] = "";
int auth_config_version = 0;

int auth_ctrl_config_reload(cJSON *uJS);
int auth_users_reload(cJSON *uJS);

#define SSID_MAX 32
static struct auth_user_hash *s_auth_user_hash[SSID_MAX];

static int auth_user_hash_init(void)
{
	memset(s_auth_user_hash, 0, sizeof(s_auth_user_hash));
	return 0;
}

static struct auth_user_hash *auth_user_hash_in(const char *ssid, int new)
{
	int i;
	struct auth_user_hash *auh;
	unsigned int hash;
	struct hlist_head *slot;


	for (i = 0; i < SSID_MAX; i++)
	{
		auh = s_auth_user_hash[i];
		if (auh == NULL)
			continue;
		if (strcmp((const char *)auh->ssid, ssid) == 0)
			return auh;
	}

	if (!new)
		return NULL;

	for (i = 0; i < SSID_MAX; i++)
	{
		auh = s_auth_user_hash[i];
		if (auh == NULL)
			break;
	}

	if (i == SSID_MAX) //XXX full
		return NULL;

	auh = malloc(sizeof(struct auth_user_hash));
	if (auh == NULL) //XXX no mem
		return NULL;
	for (hash = 0; hash < AUTH_USER_HASH_SIZE; hash++)
	{
		slot = &auh->slots[hash & AUTH_USER_HASH_MASK];
		INIT_HLIST_HEAD(slot);
	}

	memcpy(auh->ssid, ssid, 64);
	s_auth_user_hash[i] = auh;

	return auh;
}

static int auth_user_insert(const char *ssid, const char *username, const char *password, unsigned int uid, unsigned int multionline)
{
	struct auth_user_info *user = NULL;
	unsigned int hash;
	struct hlist_head *slot;
	struct hlist_node *node;
	struct auth_user_hash *auh;

	auh = auth_user_hash_in(ssid, 1);
	if (auh == NULL)
		return -1;

	hash = auth_user_hash(username, strlen(username));
	slot = &auh->slots[hash & AUTH_USER_HASH_MASK];

	hlist_for_each_entry(user, node, slot, hnode)
	{
		if (user->uid == uid)
		{
			return -1;
		}
	}

	user = malloc(sizeof(struct auth_user_info));
	if (user == NULL)
		return -1;

	memset(user, 0, sizeof(*user));
	user->uid = uid;
	user->multionline = multionline;
	memcpy(user->username, username, 64);
	memcpy(user->password, password, 64);

	hlist_add_head(&user->hnode, slot);

	return 0;
}

struct auth_user_info *auth_user_lookup(const char *ssid, const char *username)
{
	struct auth_user_info *user = NULL;
	unsigned int hash;
	struct hlist_head *slot;
	struct hlist_node *node;
	struct auth_user_hash *auh;

	auh = auth_user_hash_in(ssid, 0);
	if (auh == NULL)
		return NULL;

	hash = auth_user_hash(username, strlen(username));
	slot = &auh->slots[hash & AUTH_USER_HASH_MASK];

	hlist_for_each_entry(user, node, slot, hnode)
	{
		if (strcmp((char *)user->username, username) == 0)
		{
			return user;
		}
	}

	return NULL;
}

static void auth_config_load(const char *path)
{
	cJSON *uJS = auth_load_cJSON(path);
	if (uJS)
	{
		cJSON *js = cJSON_GetObjectItem(uJS, "ConfigVersion");
		if (js)
		{
			auth_config_version = js->valueint;
		}

		auth_ctrl_config_reload(uJS);
		auth_users_reload(uJS);
		cJSON_Delete(uJS);
	}
	else
	{
		auth_ctrl_config_reload(uJS);
	}

	update_on_connect_msg();
}

int auth_user_config_init(void)
{
	auth_user_hash_init();

	auth_config_load("/ugwconfig/etc/ap/authd.json");

	return 0;
}

static void auth_user_hash_exit(void)
{
	int i;
	struct auth_user_hash *auh;

	for (i = 0; i < SSID_MAX; i++)
	{
		auh = s_auth_user_hash[i];
		if (auh != NULL)
		{
			struct auth_user_info *user = NULL;
			unsigned int hash;
			struct hlist_head *slot;
			struct hlist_node *node, *n;

			for (hash = 0; hash < AUTH_USER_HASH_SIZE; hash++)
			{
				slot = &auh->slots[hash & AUTH_USER_HASH_MASK];
				hlist_for_each_entry_safe(user, node, n, slot, hnode) {
					hlist_del(&user->hnode);
					free(user);
				}
			}
			free(auh);
			s_auth_user_hash[i] = NULL;
		}
	}
}

void auth_user_config_exit(void)
{
	auth_ctrl_config_clear();
	auth_user_hash_exit();
}

int auth_user_config_reload()
{
	auth_user_config_exit();
	return auth_user_config_init();
}

static int __load_users_group(cJSON *uJS, const char *ssid, const char *group)
{
	int ret;
	cJSON *UsersJS;
	cJSON *GroupJS;
	int jsz;
	int j;

	UsersJS = cJSON_GetObjectItem(uJS, "Users");
	if (UsersJS == NULL)
		return -1;

	GroupJS = cJSON_GetObjectItem(UsersJS, group);
	if (GroupJS == NULL)
		return -1;

	jsz = cJSON_GetArraySize(GroupJS);

	for (j = 0; j < jsz; j++)
	{
		cJSON *UserJS = cJSON_GetArrayItem(GroupJS, j);
		if (UserJS)
		{
			cJSON *UidJS = cJSON_GetObjectItem(UserJS, "Uid");
			cJSON *PasswordJS = cJSON_GetObjectItem(UserJS, "Password");
			cJSON *MultiOnlineJS = cJSON_GetObjectItem(UserJS, "MultiOnline");

			if (UidJS && PasswordJS && MultiOnlineJS)
			{
				ret = auth_user_insert(ssid, UserJS->string, PasswordJS->valuestring, UidJS->valueint, MultiOnlineJS->valueint);
				if (ret != 0)
					return ret;
			}
		}
	}
	return 0;
}

static unsigned int get_local_ip(char *eth)
{
	int fd; 
	struct ifreq ifr;

	if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) 
	{   
		return -1; 
	}   

	strncpy(ifr.ifr_name, eth, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{   
		close(fd);
		return -1; 
	}   

	close(fd);

	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
}


static int __setup_redirect_payload(const char *ssid, const char *redirect_payload)
{
	if (redirect_payload)
	{
		return auth_ctrl_config_redirect_payload_set(ssid, redirect_payload, strlen(redirect_payload));
	}
	
	//load default redirect_payload
	unsigned int ip = get_local_ip("br0");
	ip = ntohl(ip);

	int n;
	char payload[1024];
	char host[64];
	sprintf(host, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip >> 0) & 0xFF);
	n = sprintf(payload,
			"HTTP/1.1 302 Moved Temporarily\r\n"
			"Location: http://%s/index.html?mac=%%s&ip=%%s&ssid=%%s\r\n"
			"Content-Type: text/html;\r\n"
			"Cache-Control: no-cache\r\n"
			"Content-Length: 0\r\n\r\n",
			host);

	auth_ctrl_config_bypass_host_add(ssid, host);

	return auth_ctrl_config_redirect_payload_set(ssid, payload, n);
}

static int __setup_bypass_host(const char *ssid, cJSON *BypassHostJS)
{
	int j;
	int jsz = cJSON_GetArraySize(BypassHostJS);

	for (j = 0; j < jsz; j++)
	{
		cJSON *HostJS = cJSON_GetArrayItem(BypassHostJS, j);
		if (HostJS)
		{
			auth_ctrl_config_bypass_host_add(ssid, HostJS->valuestring);
		}
	}

	return 0;
}

static int __setup_netdev_list(const char *ssid, cJSON *apJS)
{
	cJSON *WlanIdsStringJS = cJSON_GetObjectItem(apJS, "a#wlanids");
	cJSON *WlanIdsJS = cJSON_Parse(WlanIdsStringJS->valuestring);
	if (WlanIdsJS)
	{
		int j;
		int jsz = cJSON_GetArraySize(WlanIdsJS);

		for (j = 0; j < jsz; j++)
		{
			cJSON *WlanIdJS = cJSON_GetArrayItem(WlanIdsJS, j);
			if (WlanIdJS)
			{
				char key[64];
				sprintf(key, "w#%s#wssid", WlanIdJS->valuestring);
				cJSON *SsidJS = cJSON_GetObjectItem(apJS, key);
				if (strcmp(SsidJS->valuestring, ssid) == 0)
				{
					char netdev[64];
					sprintf(netdev, "ath2%03d", atoi(WlanIdJS->valuestring));
					auth_ctrl_config_netdev_list_add(ssid, netdev);
					sprintf(netdev, "ath5%03d", atoi(WlanIdJS->valuestring));
					auth_ctrl_config_netdev_list_add(ssid, netdev);
				}
			}
		}
		cJSON_Delete(WlanIdsJS);
	}
	return 0;
}

int auth_ctrl_config_reload(cJSON *uJS)
{
	int ret = 0;
	cJSON *apJS = auth_load_cJSON("/ugwconfig/etc/ap/ap_config.json");

	auth_ctrl_config_clear();

	if (apJS)
	{
		cJSON *ac_groupJS = cJSON_GetObjectItem(apJS, "a#account");
		if (ac_groupJS)
		{
			strncpy(auth_user_ac_account, ac_groupJS->valuestring, sizeof(auth_user_ac_account));
		}
	}

	if (uJS && apJS)
	{
		cJSON *AuthRulesJS = cJSON_GetObjectItem(uJS, "AuthRules");
		if (!AuthRulesJS)
		{
			ret = -1;
			goto out;
		}

		int i;
		int isz = cJSON_GetArraySize(AuthRulesJS);
		
		for (i = 0; i < isz; i++)
		{
			cJSON *SsidJS =  cJSON_GetArrayItem(AuthRulesJS, i);
			if (SsidJS)
			{
				auth_ctrl_config_create(SsidJS->string); //XXX

				cJSON *AuthTypeJS = cJSON_GetObjectItem(SsidJS, "AuthType");
				if (AuthTypeJS == NULL)
				{
					auth_ctrl_config_delete(SsidJS->string);
					continue;
				}

				if (strcmp(AuthTypeJS->valuestring, "probe") == 0)
				{
					auth_ctrl_config_auth_type_set(SsidJS->string, "probe"); //XXX

					cJSON *RedirectPayloadJS = cJSON_GetObjectItem(SsidJS, "RedirectPayload");
					if (RedirectPayloadJS)
					{
						__setup_redirect_payload(SsidJS->string, RedirectPayloadJS->valuestring);
					}
					else
					{
						__setup_redirect_payload(SsidJS->string, NULL); //XXX load default redirect_payload
					}
				}
				else if (strcmp(AuthTypeJS->valuestring, "auto") == 0)
				{
					auth_ctrl_config_auth_type_set(SsidJS->string, "auto");
				}
				else
				{
					printf("auth type [%s] not supported\n", AuthTypeJS->valuestring);
				}

				//TODO setup bypass_host
				cJSON *BypassHostJS = cJSON_GetObjectItem(SsidJS, "BypassHost");
				if (BypassHostJS)
				{
					__setup_bypass_host(SsidJS->string, BypassHostJS);
				}

				__setup_netdev_list(SsidJS->string, apJS);

				auth_ctrl_config_enable(SsidJS->string);
			}
		}
	}

out:
	if (apJS)
		cJSON_Delete(apJS);
	return ret;
}

int auth_users_reload(cJSON *uJS)
{
	int ret = 0;

	auth_user_hash_exit();

	if (uJS)
	{
		cJSON *AuthRulesJS = cJSON_GetObjectItem(uJS, "AuthRules");
		if (!AuthRulesJS)
		{
			ret = -1;
			goto out;
		}

		int i;
		int isz = cJSON_GetArraySize(AuthRulesJS);
		
		for (i = 0; i < isz; i++)
		{
			cJSON *SsidJS =  cJSON_GetArrayItem(AuthRulesJS, i);
			if (SsidJS)
			{
				cJSON *AuthTypeJS = cJSON_GetObjectItem(SsidJS, "AuthType");
				if (AuthTypeJS == NULL)
				{
					continue;
				}

				if (strcmp(AuthTypeJS->valuestring, "probe") == 0)
				{
					cJSON *UserGroupsJS =  cJSON_GetObjectItem(SsidJS, "UserGroups");
					if (UserGroupsJS)
					{
						int j;
						int jsz = cJSON_GetArraySize(UserGroupsJS);

						for (j = 0; j < jsz; j++)
						{
							cJSON *GroupJS = cJSON_GetArrayItem(UserGroupsJS, j);
							if (GroupJS)
							{
								if (__load_users_group(uJS, SsidJS->string, GroupJS->valuestring) != 0)
								{
									ret = -1;
									goto out;
								}
							}
						}
					}
				}
			}
		}
	}

out:
	return ret;
}

void update_on_connect_msg(void)
{
	cJSON *js = auth_load_cJSON("/tmp/memfile/on_connect.json");
	if (!js)
	{
		js = cJSON_CreateObject();
		if (!js) return;
	}

	cJSON *authJS = cJSON_GetObjectItem(js, "auth");

	if (!authJS)
	{
		authJS = cJSON_CreateObject();
		if (authJS)
		{
			cJSON_AddNumberToObject(authJS, "config_version", auth_config_version);
			cJSON_AddItemToObject(js, "auth", authJS);
		}
	}

	if (authJS)
	{
		cJSON *config_versionJS = cJSON_GetObjectItem(authJS, "config_version");
		if (!config_versionJS)
		{
			cJSON_AddNumberToObject(authJS, "config_version", auth_config_version);
			config_versionJS = cJSON_GetObjectItem(authJS, "config_version");
		}
		if (config_versionJS)
		{
			cJSON_SetNumberValue(config_versionJS, auth_config_version);
		}

		char *data = cJSON_Print(js);
		if (data)
		{
			printf("%s\n", data);
			auth_misc_io_store("/tmp/memfile/on_connect.json", data, strlen(data));
			free(data);
		}
	}

	cJSON_Delete(js);
}
