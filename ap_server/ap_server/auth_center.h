/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 07 Aug 2015 15:15:54 +0800
 */
#ifndef _AUTH_CENTER_H_
#define _AUTH_CENTER_H_

#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct online_user_info {
	struct hlist_node hnode;
	unsigned int uid;
	unsigned char mac[6];
};

#define ONLINE_USER_HASH_SIZE       (1 << 11)
#define ONLINE_USER_HASH_MASK       (ONLINE_USER_HASH_SIZE - 1)

struct online_user_hash {
	struct list_head node;
	time_t last_time;
	unsigned char ssid[64];
	struct hlist_head slots[ONLINE_USER_HASH_SIZE];
};

struct ap_info {
	struct list_head node;
	time_t last_time;
	unsigned char reply_topic[64];
};

extern struct ap_info *ap_info_in(const char *topic);

extern int auth_center_login_success(const char *reply_topic, const char *data);
extern int auth_center_check_online(const char *reply_topic, const char *data);

#endif /* _AUTH_CENTER_H_ */
