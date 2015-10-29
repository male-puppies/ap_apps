/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 16:10:58 +0800
 */
#ifndef _AUTH_LOCAL_SERVER_H_
#define _AUTH_LOCAL_SERVER_H_

#include "auth_misc.h"

extern int auth_local_server_fd(void);

extern int auth_local_server_init(void);

extern void auth_local_server_exit(void);

extern void auth_local_server_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd);
extern void auth_local_server_select_handle(fd_set *read_set, fd_set *write_set);
extern void auth_local_server_select_loop(void);

#endif /* _AUTH_LOCAL_SERVER_H_ */
