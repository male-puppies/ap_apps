/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 16:10:58 +0800
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
#include "auth_local_server.h"
#include "auth_misc.h"
#include "auth_mqtt.h"
#include "auth_client.h"

#define AUTH_LOCAL_SERVER_BIND_PORT (2222)
#define AUTH_LOCAL_SERVER_BIND_ADDR "127.0.0.1"

static int local_server_fd = INVALID_SOCKET;

int auth_local_server_fd(void)
{
	return local_server_fd;
}

int auth_local_server_init(void)
{
	int ret, fd;
	struct sockaddr_in si_me;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == INVALID_SOCKET)
		return -1;

	memset((char *)&si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(AUTH_LOCAL_SERVER_BIND_PORT);
	inet_pton(AF_INET, AUTH_LOCAL_SERVER_BIND_ADDR, &si_me.sin_addr);

	ret = bind(fd, (struct sockaddr *)&si_me, sizeof(si_me));
	if (ret == -1)
		return -1;

	set_non_blocking_mode(fd);

	if (local_server_fd != INVALID_SOCKET)
	{
		close(local_server_fd);
		local_server_fd = INVALID_SOCKET;
	}
	local_server_fd = fd;

	return 0;
}

void auth_local_server_exit(void)
{
	if (local_server_fd != INVALID_SOCKET)
	{
		close(local_server_fd);
		local_server_fd = INVALID_SOCKET;
	}
}

void auth_local_server_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd)
{
	write_set = write_set; //XXX ignore warning

	int fd = auth_local_server_fd();
	if (fd != INVALID_SOCKET)
	{
		FD_SET(fd, read_set);
		if (*max_fd < fd)
		{
			*max_fd = fd;
		}
	}
}

void auth_local_server_select_handle(fd_set *read_set, fd_set *write_set)
{
	write_set = write_set; //XXX ignore warning

	int fd = auth_local_server_fd();
	if (fd != INVALID_SOCKET) 
	{
		if (FD_ISSET(fd, read_set))
		{
			int n;
			char buf[2000];
			struct sockaddr_in si_other;
			socklen_t slen = sizeof(si_other);

			n = recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&si_other, &slen);
			if (n == -1)
			{
				auth_local_server_exit();
				return;
			}

			struct auth_request_data *req = (struct auth_request_data *)buf;

			if ((req->flags & AUTH_USER_LOGIN_REQUEST))
			{
				struct auth_user_info *user = auth_user_lookup(req->ssid, req->username);
				printf("user=%p, %s %s\n", user, req->ssid, req->username);
				if (user)
				{
					if (strcmp((char *)user->password, req->password) == 0)
					{
						if (AUTH_USER_SET_ONLINE(req->mac, req->ssid, user->uid) == 0)
						{
							req->flags |= AUTH_USER_LOGIN_SUCCESS;
							auth_client_login_publish(req->mac, req->ssid, user->uid);
						}
					}
				}
			}
			else if ((req->flags & AUTH_USER_ONLINE_REQUEST))
			{
				printf("check online ssid=%s\n", req->ssid);
				if (AUTH_USER_CHECK_ONLINE(req->mac, req->ssid) == 0)
					req->flags |= AUTH_USER_ONLINE_SUCCESS;
			}

			n = sendto(fd, buf, n, 0, (struct sockaddr*) &si_other, slen);
			if (n == -1)
			{
				auth_local_server_exit();
				return;
			}
		}
	}
}

void auth_local_server_select_loop(void)
{
	if (auth_local_server_fd() == INVALID_SOCKET)
	{
		auth_local_server_init();
	}
}
