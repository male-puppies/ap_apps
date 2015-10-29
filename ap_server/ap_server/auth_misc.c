/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Thu, 06 Aug 2015 16:31:07 +0800
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stddef.h>     // required for size_t
#include <stdarg.h>
#include "auth_misc.h"

#define AUTH_CTRL_USERS_PATH "/sys/module/auth/ctrl/users"
static int auth_ctrl_users_fd = INVALID_SOCKET;

static int auth_ctrl_users_fd_init(void)
{
	int fd;

	if (auth_ctrl_users_fd == INVALID_SOCKET)
	{
		fd = open(AUTH_CTRL_USERS_PATH, O_RDWR);
		if (fd == INVALID_SOCKET)
		{
			auth_ctrl_users_fd = INVALID_SOCKET;
			return -1;
		}
		auth_ctrl_users_fd = fd;
	}

	return 0;
}

#define __AUTH_CTRL_USERS_PRE_INIT do { \
	if (auth_ctrl_users_fd_init() != 0) return -1; \
} while (0)

#define __AUTH_CTRL_USERS_ON_FAIL do { \
	if (auth_ctrl_users_fd != INVALID_SOCKET) { \
		close(auth_ctrl_users_fd); \
		auth_ctrl_users_fd = INVALID_SOCKET; \
	} \
} while (0)

int auth_ctrl_users_write(const char *buf, size_t len)
{
	__AUTH_CTRL_USERS_PRE_INIT;

	ssize_t n = write(auth_ctrl_users_fd, buf, len);
	if (n == -1)
	{
		__AUTH_CTRL_USERS_ON_FAIL;
		return -1;
	}

	return (int)n;
}

int auth_ctrl_users_read(char *buf, size_t count)
{
	__AUTH_CTRL_USERS_PRE_INIT;

	ssize_t n = read(auth_ctrl_users_fd, buf, count);
	if (n == -1)
	{
		__AUTH_CTRL_USERS_ON_FAIL;
		return -1;
	}

	return (int)n;
}

int auth_ctrl_users_add(unsigned char *mac, char *ssid, unsigned long status, unsigned int uid)
{
	char sys_cmd[128];

	int n = sprintf(sys_cmd, "add %02X:%02X:%02X:%02X:%02X:%02X,%s,%lX,%X\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
			ssid,
			status,
			uid);

	n = auth_ctrl_users_write(sys_cmd, n);

	if (n == -1)
		return -1;

	return 0;
}

int auth_ctrl_users_checkonline(unsigned char *mac, char *ssid)
{
	char sys_cmd[128];

	int n = sprintf(sys_cmd, "checkonline %02X:%02X:%02X:%02X:%02X:%02X,%s\n",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
			ssid);

	printf("%s\n", sys_cmd);
	n = auth_ctrl_users_write(sys_cmd, n);

	if (n == -1)
		return -1;

	return 0;
}


#define AUTH_CTRL_SWITCH_PATH "/sys/module/auth/ctrl/switch"
int auth_ctrl_config_create(const char *ssid)
{
	int n;
	int fd;
	char sys_cmd[128];

	fd = open(AUTH_CTRL_SWITCH_PATH, O_RDWR);
	if (fd == INVALID_SOCKET)
		return -1;

	n = sprintf(sys_cmd, "create %s\n", ssid);

	n = write(fd, sys_cmd, n);

	if (n == -1)
	{
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int auth_ctrl_config_delete(const char *ssid)
{
	int n;
	int fd;
	char sys_cmd[128];

	fd = open(AUTH_CTRL_SWITCH_PATH, O_RDWR);
	if (fd == INVALID_SOCKET)
		return -1;

	n = sprintf(sys_cmd, "delete %s\n", ssid);

	n = write(fd, sys_cmd, n);

	if (n == -1)
	{
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int auth_ctrl_config_clear(void)
{
	int n;
	int fd;
	char sys_cmd[128];

	fd = open(AUTH_CTRL_SWITCH_PATH, O_RDWR);
	if (fd == INVALID_SOCKET)
		return -1;

	n = sprintf(sys_cmd, "clear\n");

	n = write(fd, sys_cmd, n);

	if (n == -1)
	{
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

static int __auth_ctrl_config_write(const char *ssid, const char *token, const void *data, int len)
{
	int n;
	int fd;
	char path[512];

	n = sprintf(path, "/sys/module/auth/%s/%s", ssid, token);

	fd = open(path, O_RDWR);
	if (fd == INVALID_SOCKET)
		return -1;

	n = write(fd, data, len);
	
	if (n == -1)
	{
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int auth_ctrl_config_redirect_payload_set(const char *ssid, const void *payload, int payload_len)
{
	return __auth_ctrl_config_write(ssid, "redirect_payload", payload, payload_len);
}

int auth_ctrl_config_bypass_host_add(const char *ssid, const char *host)
{
	int n;
	char sys_cmd[1024];
	n = sprintf(sys_cmd, "add %s\n", host);
	return __auth_ctrl_config_write(ssid, "bypass_host", sys_cmd, n);
}

int auth_ctrl_config_bypass_host_remove(const char *ssid, const char *host)
{
	int n;
	char sys_cmd[1024];
	n = sprintf(sys_cmd, "remove %s\n", host);
	return __auth_ctrl_config_write(ssid, "bypass_host", sys_cmd, n);
}

int auth_ctrl_config_bypass_host_clear(const char *ssid)
{
	return __auth_ctrl_config_write(ssid, "bypass_host", "clear\n", 6);
}

int auth_ctrl_config_netdev_list_add(const char *ssid, const char *netdev)
{
	int n;
	char sys_cmd[1024];
	n = sprintf(sys_cmd, "add %s\n", netdev);
	return __auth_ctrl_config_write(ssid, "netdev_list", sys_cmd, n);
}

int auth_ctrl_config_netdev_list_remove(const char *ssid, const char *netdev)
{
	int n;
	char sys_cmd[1024];
	n = sprintf(sys_cmd, "remove %s\n", netdev);
	return __auth_ctrl_config_write(ssid, "netdev_list", sys_cmd, n);
}

int auth_ctrl_config_netdev_list_clear(const char *ssid)
{
	return __auth_ctrl_config_write(ssid, "netdev_list", "clear\n", 6);
}

int auth_ctrl_config_auth_type_set(const char *ssid, const char *auth_type)
{
	return __auth_ctrl_config_write(ssid, "auth_type", auth_type, strlen(auth_type));
}

int auth_ctrl_config_enable(const char *ssid)
{
	return __auth_ctrl_config_write(ssid, "auth_bypass_enable", "0\n", 2);
}

int auth_ctrl_config_disable(const char *ssid)
{
	return __auth_ctrl_config_write(ssid, "auth_bypass_enable", "1\n", 2);
}

int auth_misc_io_store(const char *fpath, const void *data, int data_len)
{
	char tmp[4096];
	FILE *fp;

	fp = NULL;

	(void) snprintf(tmp, sizeof(tmp), "%s.tmp", fpath);

	// Open the given file and temporary file
	if ((fp = fopen(tmp, "w+")) == NULL) {
		return -1; 
	}

	fwrite(data, 1, data_len, fp);

	// Close files
	fclose(fp);

	remove(fpath);
	rename(tmp, fpath);

	return 0;
}
