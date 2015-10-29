/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Tue, 21 Jul 2015 15:51:59 +0800
 */
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <arpa/inet.h>

#include "cJSON.h"
#include "mosquitto.h"
#include "websrv.h"
#include "auth_utils.h"
#include "auth_main.h"
#include "auth_user.h"
#include "auth_misc.h"
#include "auth_local_server.h"
#include "auth_mqtt.h"
#include "auth_client.h"
#include "auth_user_request_queue.h"

static int s_received_signal = 0;

static void signal_handler(int sig_num) {
	signal(sig_num, signal_handler);
	s_received_signal = sig_num;
}

int main(void)
{
	int ret;
	struct timespec ts;
	fd_set read_set, write_set;
	int max_fd = INVALID_SOCKET;
	time_t current_time, last_time;

	auth_user_config_init();
	auth_local_server_init();
	auth_user_request_queue_init();
	auth_mqtt_init();

	// Setup signal handlers
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);

	current_time = last_time = get_current_time();
	while (s_received_signal == 0)
	{
		current_time = get_current_time();
		max_fd = INVALID_SOCKET;
		ts.tv_sec = 1;
		ts.tv_nsec = 0;
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);

		auth_local_server_select_setup(&read_set, &write_set, &max_fd);

		auth_mqtt_select_setup(&read_set, &write_set, &max_fd);

		auth_user_request_queue_select_setup(&read_set, &write_set, &max_fd);

		ret = pselect(max_fd + 1, &read_set, &write_set, NULL, &ts, NULL);
		if (ret > 0)
		{
			auth_local_server_select_handle(&read_set, &write_set);

			auth_mqtt_select_handle(&read_set, &write_set);

			auth_user_request_queue_select_handle(&read_set, &write_set);
		}

		auth_local_server_select_loop();

		auth_mqtt_select_loop();

		auth_user_request_queue_select_loop();

		auth_client_check_server(current_time);

		last_time = current_time;
	}

	printf("Existing on signal %d\n", s_received_signal);

	auth_mqtt_exit();
	auth_user_request_queue_exit();
	auth_local_server_exit();
	auth_user_config_exit();

	return 0;
}

