#include "ap_ioctl.h"

int main()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return -1;
	
	char if_name[] = "ath01";
	int curr_power = 0;
	int err = get_curr_power(sock, if_name, &curr_power);
	
	if (err != 0)
	{
		printf("%s\n", strerror(err));
	}
	else
	{
		printf("curr_power:%d\n", curr_power);
	}
		
	if (sock > 0)
		close(sock);

	return 0;
}