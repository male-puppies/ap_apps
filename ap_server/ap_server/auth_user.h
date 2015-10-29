/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 24 Jul 2015 15:15:01 +0800
 */
#ifndef _AUTH_USER_H_
#define _AUTH_USER_H_

#include "cJSON.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct auth_user_info {
	struct hlist_node hnode;
	unsigned int multionline;
	unsigned int uid;
	unsigned char username[64];
	unsigned char password[64];
};

#define AUTH_USER_HASH_SIZE       (1 << 11)
#define AUTH_USER_HASH_MASK       (AUTH_USER_HASH_SIZE - 1)

struct auth_user_hash {
	unsigned char ssid[64];
	struct hlist_head slots[AUTH_USER_HASH_SIZE];
};

static inline unsigned int auth_user_hash(const char *username, size_t len)
{
	size_t i;
	unsigned int hash = 0;

	for (i = 0; i < len; i++)
	{
		hash += hash * 33 + username[i];
	}

	return hash;
}

extern int auth_user_config_init(void);
extern void auth_user_config_exit(void);
extern int auth_user_config_reload(void);
extern struct auth_user_info *auth_user_lookup(const char *ssid, const char *username);

extern char auth_user_ac_account[64];
extern int auth_config_version;

extern int auth_ctrl_config_reload(cJSON *uJS);
extern int auth_users_reload(cJSON *uJS);

extern void update_on_connect_msg(void);

#endif /* _AUTH_USER_H_ */
