/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 07 Aug 2015 15:15:54 +0800
 */
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "auth_utils.h"
#include "auth_center.h"
#include "auth_misc.h"
#include "auth_mqtt.h"
#include "cJSON.h"

static LIST_HEAD(ap_info_list_head);

static int ap_info_store(void)
{
	struct ap_info *ap;
	cJSON *apsJS;
	cJSON *apJS;
	char *apsStr;

	apsJS = cJSON_CreateArray();

	list_for_each_entry(ap, &ap_info_list_head, node)
	{
		apJS = cJSON_CreateObject();
		cJSON_AddStringToObject(apJS, "reply_topic", (const char *)ap->reply_topic);
		cJSON_AddNumberToObject(apJS, "last_time", (unsigned long)ap->last_time);
		cJSON_AddItemToArray(apsJS, apJS);
	}

	apsStr = cJSON_Print(apsJS);

	auth_misc_io_store("/tmp/auth_center_ap_list", apsStr, strlen(apsStr));

	free(apsStr);
	cJSON_Delete(apsJS);

	return 0;
}

static struct ap_info *ap_info_lookup(const char *topic)
{
	struct ap_info *ap;

	list_for_each_entry(ap, &ap_info_list_head, node)
	{
		if (strcmp(topic, (char *)ap->reply_topic) == 0)
			return ap;
	}

	return NULL;
}

static struct ap_info *ap_info_insert(const char *topic)
{
	struct ap_info *ap = malloc(sizeof(struct ap_info));

	if (!ap)
		return NULL;

	INIT_LIST_HEAD(&ap->node);
	memcpy(ap->reply_topic, topic, 64);
	ap->last_time = get_current_time();
	list_add(&ap->node, &ap_info_list_head);
	ap_info_store();
	return ap;
}

struct ap_info *ap_info_in(const char *topic)
{
	struct ap_info *ap;

	if (topic == NULL)
		return NULL;

	ap = ap_info_lookup(topic);
	if (ap)
	{
		ap->last_time = get_current_time();
		ap_info_store();
		return ap;
	}

	return ap_info_insert(topic);
}

static LIST_HEAD(online_user_hash_list_head);

static struct online_user_hash *online_user_hash_lookup(const char *ssid)
{
	struct online_user_hash *ouh;

	list_for_each_entry(ouh, &online_user_hash_list_head, node)
	{
		if (strcmp(ssid, (char *)ouh->ssid) == 0)
			return ouh;
	}

	return NULL;
}

static struct online_user_hash *online_user_hash_insert(const char *ssid)
{
	struct online_user_hash *ouh = malloc(sizeof(struct online_user_hash));

	if (!ouh)
		return NULL;

	memset(ouh, 0, sizeof(struct online_user_hash));
	INIT_LIST_HEAD(&ouh->node);
	memcpy(ouh->ssid, ssid, 64);
	ouh->last_time = get_current_time();
	list_add(&ouh->node, &online_user_hash_list_head);
	return ouh;
}

struct online_user_hash *online_user_hash_in(const char *ssid, int new)
{
	struct online_user_hash *ouh;

	if (ssid == NULL)
		return NULL;

	ouh = online_user_hash_lookup(ssid);
	if (ouh)
	{
		ouh->last_time = get_current_time();
		return ouh;
	}

	if (!new)
		return NULL;

	return online_user_hash_insert(ssid);
}

static inline unsigned int online_user_info_hash(const unsigned char *mac, size_t len)
{
	size_t i;
	unsigned int hash = 0;

	for (i = 0; i < len; i++)
	{
		hash += hash * 33 + mac[i];
	}

	return hash;
}

struct online_user_info *online_user_info_lookup(const char *ssid, const unsigned char *mac)
{
	struct online_user_info *user = NULL;
	unsigned int hash;
	struct hlist_head *slot;
	struct hlist_node *node;
	struct online_user_hash *ouh;

	ouh = online_user_hash_in(ssid, 0);
	if (ouh == NULL)
		return NULL;

	hash = online_user_info_hash(mac, 6);
	slot = &ouh->slots[hash & ONLINE_USER_HASH_MASK];

	hlist_for_each_entry(user, node, slot, hnode)
	{
		if (memcmp(user->mac, mac, 6) == 0)
		{
			return user;
		}
	}

	return NULL;
}

struct online_user_info *online_user_info_insert(const char *ssid, const unsigned char *mac, unsigned int uid)
{
	struct online_user_info *user = NULL;
	unsigned int hash;
	struct hlist_head *slot;
	struct hlist_node *node;
	struct online_user_hash *ouh;

	ouh = online_user_hash_in(ssid, 1);
	if (ouh == NULL)
		return NULL;

	hash = online_user_info_hash(mac, 6);
	slot = &ouh->slots[hash & ONLINE_USER_HASH_MASK];

	hlist_for_each_entry(user, node, slot, hnode)
	{
		if (memcmp(user->mac, mac, 6) == 0)
		{
			user->uid = uid;
			return user;
		}
	}

	user = malloc(sizeof(struct online_user_info));
	if (user == NULL)
		return NULL;

	memset(user, 0, sizeof(struct online_user_info));
	user->uid = uid;
	memcpy(user->mac, mac, 6);

	hlist_add_head(&user->hnode, slot);

	return user;
}

int auth_center_login_success(const char *reply_topic, const char *data)
{
	reply_topic = reply_topic;
	int n;
	unsigned char macaddr[6];
	unsigned int mac[6];
	char ssid[64];
	unsigned int uid;

	memset(ssid, 0, sizeof(ssid));
	n = sscanf(data, "%02X:%02X:%02X:%02X:%02X:%02X,%[^,],%X\n",
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

	online_user_info_insert(ssid, macaddr, uid);

	return 0;
}

int auth_center_check_online(const char *reply_topic, const char *data)
{
	int n;
	unsigned char macaddr[6];
	unsigned int mac[6];
	char ssid[64];

	printf("CENTER:: %s\n", data);

	memset(ssid, 0, sizeof(ssid));
	n = sscanf(data, "%02X:%02X:%02X:%02X:%02X:%02X,%[^\n^,]\n",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
			(char *)&ssid);

	if (n != 7)
	{
		return -1;
	}

	macaddr[0] = mac[0];
	macaddr[1] = mac[1];
	macaddr[2] = mac[2];
	macaddr[3] = mac[3];
	macaddr[4] = mac[4];
	macaddr[5] = mac[5];

	struct online_user_info *oui = online_user_info_lookup(ssid, macaddr);

	if (oui != NULL)
	{
		char buf[128];
		cJSON *msg;
		char *msgstr;

		sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X,%s,%X\n",
				oui->mac[0], oui->mac[1], oui->mac[2], oui->mac[3], oui->mac[4], oui->mac[5],
				ssid, oui->uid);

		msg = cJSON_CreateObject();
		cJSON_AddStringToObject(msg, "reply_topic", "a/local/auth");
		//cJSON_AddNumberToObject(msg, "reply_seq", 2);
		cJSON_AddStringToObject(msg, "cmd", "set_online");
		cJSON_AddStringToObject(msg, "data", buf);

		msgstr = cJSON_PrintUnformatted(msg);
		printf("%s\n", msgstr);

		auth_mqtt_publish(reply_topic, msgstr, strlen(msgstr));
		free(msgstr);
		cJSON_Delete(msg);
	}

	return 0;
}
