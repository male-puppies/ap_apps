/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 17:49:43 +0800
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#include "cJSON.h"
#include "mosquitto.h"
#include "websrv.h"
#include "auth_user.h"
#include "auth_misc.h"
#include "auth_mqtt.h"
#include "auth_client.h"
#include "auth_user_request_queue.h"

#define AUTH_USER_REQUEST_QUEUE_PATH "/dev/user_request_queue"
static int user_request_queue_fd = INVALID_SOCKET;

int auth_user_request_queue_fd(void)
{
	return user_request_queue_fd;
}

int auth_user_request_queue_init(void)
{
	if (user_request_queue_fd != INVALID_SOCKET)
	{
		close(user_request_queue_fd);
		user_request_queue_fd = INVALID_SOCKET;
	}

	user_request_queue_fd = open(AUTH_USER_REQUEST_QUEUE_PATH, O_RDONLY);
	if (user_request_queue_fd != INVALID_SOCKET) 
	{   
		set_non_blocking_mode(user_request_queue_fd);
	} 

	return 0;
}

void auth_user_request_queue_exit(void)
{
	if (user_request_queue_fd != INVALID_SOCKET)
	{
		close(user_request_queue_fd);
		user_request_queue_fd = INVALID_SOCKET;
	}
}


void auth_user_request_queue_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd)
{
	write_set = write_set; //XXX ignore warning
	int fd = auth_user_request_queue_fd();
	if (fd != INVALID_SOCKET)
	{
		FD_SET(fd, read_set);
		if (*max_fd < fd)
		{
			*max_fd = fd;
		}
	}
}

void auth_user_request_queue_select_handle(fd_set *read_set, fd_set *write_set)
{
	write_set = write_set; //XXX ignore warning
	int ret;
	int fd = auth_user_request_queue_fd();
	if (fd != INVALID_SOCKET)
	{
		if (FD_ISSET(fd, read_set))
		{
			char buf[4096];
			ret = read(fd, buf, 4096);
			if (ret >= 0)
			{
				int n;
				buf[ret] = 0;
				unsigned int ip;
				unsigned int a, b, c, d;
				unsigned int mac[6];
				unsigned char macaddr[6];
				char ssid[64];
				unsigned long status;
				unsigned int uid;

				printf("msg %s\n", buf);

				memset(ssid, 0, sizeof(ssid));
				n = sscanf(buf, "%u.%u.%u.%u,%02X:%02X:%02X:%02X:%02X:%02X,%[^,],%lX,%X",
						&a, &b, &c, &d,
						&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
						(char *)&ssid, &status, &uid);

				if (n != 13)
				{
					return;
				}

				ip = htonl((a << 24) | (b << 16) | (c << 8) | d);
				macaddr[0] = mac[0];
				macaddr[1] = mac[1];
				macaddr[2] = mac[2];
				macaddr[3] = mac[3];
				macaddr[4] = mac[4];
				macaddr[5] = mac[5];

				printf("new user:: %u.%u.%u.%u,%02X:%02X:%02X:%02X:%02X:%02X,%s,%lX,%X\n",
						a, b, c, d, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5], ssid, status, uid);

				if (status == AUTH_NONE)
				{
					auth_client_check_roaming(macaddr, ssid);
				}
				else if (status == AUTH_TIMEOUT)
				{
					//FIXME go offline
				}
			}
			else
			{
				auth_user_request_queue_exit();
			}
		}
	}
}

void auth_user_request_queue_select_loop(void)
{
	if (auth_user_request_queue_fd() == INVALID_SOCKET)
	{
		auth_user_request_queue_init();
	}
}

