/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 16:58:45 +0800
 */
#ifndef _AUTH_MQTT_H_
#define _AUTH_MQTT_H_

extern int auth_mqtt_fd(void);

extern int auth_mqtt_init(void);

extern void auth_mqtt_exit(void);

extern void auth_mqtt_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd);
extern void auth_mqtt_select_handle(fd_set *read_set, fd_set *write_set);
extern void auth_mqtt_select_loop(void);

extern void auth_mqtt_publish(const char *topic, const void *data, int data_len);

#endif /* _AUTH_MQTT_H_ */
