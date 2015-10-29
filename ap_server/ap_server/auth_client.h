/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 21 Aug 2015 14:39:44 +0800
 */
#ifndef _AUTH_CLIENT_H_
#define _AUTH_CLIENT_H_

extern int auth_client_set_online(const char *data);

extern int auth_client_check_roaming(const unsigned char *mac, const char *ssid);

extern int auth_client_login_publish(const unsigned char *mac, const char *ssid, unsigned int uid);

extern int auth_client_check_server(time_t current_time);

#endif /* _AUTH_CLIENT_H_ */
