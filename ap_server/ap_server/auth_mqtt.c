/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 16:58:45 +0800
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
#include "auth_center.h"
#include "auth_client.h"

static struct mosquitto *auth_mqtt_mosq = NULL;

void connect_callback(struct mosquitto *mosq, void *obj, int result);
void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

int auth_mqtt_fd(void)
{
	if (auth_mqtt_mosq)
		return mosquitto_socket(auth_mqtt_mosq);
	return -1;
}

int auth_mqtt_init(void)
{
	mosquitto_lib_init();
	auth_mqtt_mosq = mosquitto_new("auth_client_XXXXXXXXXXXX", true, NULL);
	if (!auth_mqtt_mosq) {
		mosquitto_lib_cleanup();
		return -1;
	}
	mosquitto_username_pw_set(auth_mqtt_mosq, "#qmsw2..5#", "@oawifi15%");
	mosquitto_connect_callback_set(auth_mqtt_mosq, connect_callback);
	mosquitto_message_callback_set(auth_mqtt_mosq, message_callback);
	mosquitto_connect(auth_mqtt_mosq, "127.0.0.1", 1883, 60);

	return 0;
}

void auth_mqtt_exit(void)
{
	mosquitto_lib_cleanup();
}

void auth_mqtt_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd)
{
	int fd = auth_mqtt_fd();
	if (fd != INVALID_SOCKET)
	{
		FD_SET(fd, read_set);
		if (*max_fd < fd)
		{
			*max_fd = fd;
		}

		if (mosquitto_want_write(auth_mqtt_mosq))
		{
			FD_SET(fd, write_set);
		}
	}
}

void auth_mqtt_select_handle(fd_set *read_set, fd_set *write_set)
{
	int fd = auth_mqtt_fd();
	if (fd != INVALID_SOCKET)
	{
		if (FD_ISSET(fd, read_set))
		{
			if (mosquitto_loop_read(auth_mqtt_mosq, 1) != 0)
				return;
		}
		if (FD_ISSET(fd, write_set))
		{
			if (mosquitto_loop_write(auth_mqtt_mosq, 1) != 0)
				return;
		}
	}
}

void auth_mqtt_select_loop(void)
{
	int ret = mosquitto_loop_misc(auth_mqtt_mosq);
	if (ret)
	{
		mosquitto_reconnect(auth_mqtt_mosq);
	}
}

void connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	printf("connect_callback obj=%p, result=%d\n", obj, result);
	mosquitto_subscribe(mosq, NULL, "a/local/auth", 0);//client
	mosquitto_subscribe(mosq, NULL, "a/vlan/group/center", 0);//center
}

void auth_mqtt_publish(const char *topic, const void *data, int data_len)
{
	mosquitto_publish(auth_mqtt_mosq, NULL, topic, data_len, data, 0, false);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	printf("mosq=%p obj=%p top:[%s] payload: %s\n", mosq, obj, (char *)message->topic, (char *)message->payload);
	if (strcmp("a/vlan/group/center", (char *)message->topic) == 0)
	{
		printf("CENTER:\n");
		cJSON *msgJS = cJSON_Parse((char *)message->payload);

		if (msgJS)
		{
			cJSON *reply_topicJS = cJSON_GetObjectItem(msgJS, "reply_topic");
			cJSON *dataJS = cJSON_GetObjectItem(msgJS, "data");
			cJSON *mymsgJS = cJSON_Parse(dataJS->valuestring);

			ap_info_in(reply_topicJS->valuestring);

			if (mymsgJS)
			{
				//cJSON *reply_topicJS = cJSON_GetObjectItem(mymsgJS, "reply_topic");
				cJSON *cmdJS = cJSON_GetObjectItem(mymsgJS, "cmd");
				cJSON *varJS = cJSON_GetObjectItem(mymsgJS, "data");

				if (reply_topicJS && cmdJS && varJS)
				{
					if (strcmp(cmdJS->valuestring, "login_success") == 0)
					{
						auth_center_login_success(reply_topicJS->valuestring, varJS->valuestring);
					}
					else if (strcmp(cmdJS->valuestring, "check_online") == 0)
					{
						auth_center_check_online(reply_topicJS->valuestring, varJS->valuestring);
					}
				}

				cJSON_Delete(mymsgJS);
			}

			cJSON_Delete(msgJS);
		}
	}
	else if (strcmp("a/local/auth", (char *)message->topic) == 0)
	{
		printf("CLIENT:\n");
		cJSON *msgJS = cJSON_Parse((char *)message->payload);
		if (msgJS)
		{
			//cJSON *reply_topicJS = cJSON_GetObjectItem(msgJS, "reply_topic");
			cJSON *cmdJS = cJSON_GetObjectItem(msgJS, "cmd");
			cJSON *varJS = cJSON_GetObjectItem(msgJS, "data");
			if (cmdJS && varJS)
			{
				if (strcmp(cmdJS->valuestring, "set_online") == 0)
				{
					auth_client_set_online(varJS->valuestring);
				}
			}
			else
			{
				cJSON *js = cJSON_GetObjectItem(msgJS, "pld");
				cmdJS = cJSON_GetObjectItem(js, "cmd");
				varJS = cJSON_GetObjectItem(js, "data");
				if (cmdJS && varJS)
				{
					if (strcmp(cmdJS->valuestring, "push_config") == 0)
					{
						auth_client_push_config(varJS->valuestring);
					}
				}
			}
		}
	}
}
