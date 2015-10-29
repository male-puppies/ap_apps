/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Thu, 06 Aug 2015 16:31:07 +0800
 */
#ifndef _AUTH_MISC_H_
#define _AUTH_MISC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "cJSON.h"

#define INVALID_SOCKET (-1)

static inline void set_non_blocking_mode(int sock)
{
	int flags = fcntl(sock, F_GETFL, 0); 
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

enum auth_user_status {
	AUTH_NONE = 0,
	AUTH_SUCCESS = (1 << 0), 
	AUTH_TIMEOUT = (1 << 1), 
};

extern int auth_ctrl_users_add(unsigned char *mac, char *ssid, unsigned long status, unsigned int uid);
#define AUTH_USER_SET_ONLINE(mac, ssid, uid) auth_ctrl_users_add(mac, ssid, AUTH_SUCCESS, uid)

extern int auth_ctrl_users_checkonline(unsigned char *mac, char *ssid);
#define AUTH_USER_CHECK_ONLINE(mac, ssid) auth_ctrl_users_checkonline(mac, ssid)


int auth_ctrl_config_create(const char *ssid);

int auth_ctrl_config_delete(const char *ssid);

int auth_ctrl_config_clear(void);

int auth_ctrl_config_redirect_payload_set(const char *ssid, const void *payload, int payload_len);

int auth_ctrl_config_bypass_host_add(const char *ssid, const char *host);

int auth_ctrl_config_bypass_host_remove(const char *ssid, const char *host);

int auth_ctrl_config_bypass_host_clear(const char *ssid);

int auth_ctrl_config_netdev_list_add(const char *ssid, const char *netdev);

int auth_ctrl_config_netdev_list_remove(const char *ssid, const char *netdev);

int auth_ctrl_config_netdev_list_clear(const char *ssid);

int auth_ctrl_config_auth_type_set(const char *ssid, const char *auth_type);

int auth_ctrl_config_enable(const char *ssid);

int auth_ctrl_config_disable(const char *ssid);

static inline cJSON *auth_load_cJSON(const char *filepath)
{
	int fd;
	char *buf, *p;
	ssize_t n, sz = 4096;
	cJSON *userJSON = NULL;

	if (access(filepath, R_OK|F_OK))
	{
		//fprintf(stderr, "load file %s fail: %s\n", filepath, strerror(errno));
		return NULL;
	}

	fd = open(filepath, O_RDONLY);
	if (fd == INVALID_SOCKET)
		return NULL;

	buf = malloc(sz);
	p = buf;
	n = 0;
	do {
		if (!buf)
		{
			//fprintf(stderr, "realloc(size %d) fail: %s\n", sz, strerror(errno));
			break;
		}
		n = read(fd, p, sz - (p - buf));
		if (n == sz - (p - buf))
		{
			sz = sz * 2;
			buf = realloc(buf, sz);
			p = buf + sz / 2;
			continue;
		}
		else if (n < 0)
		{
			//fprintf(stderr, "read fail: %s\n", strerror(errno));
			free(buf);
			buf = NULL;
			break;
		}
		else
		{
			p = p + n;
			*p = '\0';
			n = p - buf;
			break;
		}
	} while (1);

	close(fd);

	if (!buf)
		return NULL;

	userJSON = cJSON_Parse(buf);
	free(buf);

	return userJSON;
}


extern int auth_misc_io_store(const char *fpath, const void *data, int data_len);

#endif /* _AUTH_MISC_H_ */
