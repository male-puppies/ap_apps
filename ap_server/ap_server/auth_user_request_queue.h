/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Sat, 08 Aug 2015 17:49:43 +0800
 */
#ifndef _AUTH_USER_REQUEST_QUEUE_H_
#define _AUTH_USER_REQUEST_QUEUE_H_

extern int auth_user_request_queue_fd(void);

extern int auth_user_request_queue_init(void);

extern void auth_user_request_queue_exit(void);

extern void auth_user_request_queue_select_setup(fd_set *read_set, fd_set *write_set, int *max_fd);

extern void auth_user_request_queue_select_handle(fd_set *read_set, fd_set *write_set);

extern void auth_user_request_queue_select_loop(void);

#endif /* _AUTH_USER_REQUEST_QUEUE_H_ */
