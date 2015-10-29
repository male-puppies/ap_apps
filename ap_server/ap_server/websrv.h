/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Mon, 20 Jul 2015 11:23:35 +0800
 */
#ifndef _WEBSRV_H_
#define _WEBSRV_H_

#define AUTH_USER_LOGIN_REQUEST (1 << 0)
#define AUTH_USER_ONLINE_REQUEST (1 << 1)

#define AUTH_USER_LOGIN_SUCCESS (1 << 8)
#define AUTH_USER_ONLINE_SUCCESS (1 << 9)

struct auth_request_data {
  unsigned long id;
  unsigned long flags;
  unsigned int ip;
  unsigned char mac[6];
  char username[64];
  char password[64];
  char ssid[64];
};

struct auth_response_data {
  unsigned long id;
  unsigned long flags;
};

enum {
  WEB_AUTH_INDEX,
  WEB_AUTH_LOGIN_REQUEST,
  WEB_AUTH_ONLINE_REQUEST,
  WEB_AUTH_ADMIN_REQUEST,
  WEB_AUTH_DONE,
};

struct auth_conn_param {
  unsigned long status;
  time_t time;
  union {
    struct auth_request_data req;
    struct auth_response_data rsp;
  };
};

#define AUTH_REQUEST_TIMEOUT_SECONDS 30

#endif /* _WEBSRV_H_ */
