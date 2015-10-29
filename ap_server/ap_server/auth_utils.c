/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 25 Sep 2015 16:40:10 +0800
 */
#include "auth_utils.h"

time_t get_current_time(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec;
}


