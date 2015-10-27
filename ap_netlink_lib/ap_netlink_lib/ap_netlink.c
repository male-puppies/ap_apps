#include "ap_netlink.h"


static int sock_fd = -1;
static struct nlmsghdr *nlh_recv = NULL;
static struct nlmsghdr *nlh_send = NULL;


int ugw_netlink_sock()
{
    if (-1 == sock_fd)
    {
        ugw_netlink_init();
    }

    return sock_fd;
}


int ugw_netlink_init()
{
    ugw_netlink_destroy();
    
	struct sockaddr_nl src_addr;
	
	if (sock_fd > 0)
		return 0;
	
	sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_WIRELESS);
	if (-1 == sock_fd)
	{
		printf("create socket failed:%s\n", strerror(errno));
		return errno;
	}
	
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();             // self pid
    src_addr.nl_groups = NETLINK_GROUP_ID;  // multi cast
	
	int ret = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    if(ret < 0)
	{
        printf("bind failed:%s\n", strerror(errno));
		ugw_netlink_destroy();
        return errno;
    }

    if (NULL == nlh_recv)
    {
        nlh_recv = (struct nlmsghdr *)malloc(MAX_PAYLOAD);
        if (NULL == nlh_recv)
        {
            return errno;
        }
    }

    if (NULL == nlh_send)
    {
        nlh_send = (struct nlmsghdr *)malloc(MAX_PAYLOAD);
        if (NULL == nlh_send)
        {
            return errno;
        }
    }
    
	return 0;
}


int ugw_netlink_destroy()
{
	if (sock_fd > 0)
	{
		close(sock_fd);
		sock_fd = -1;
	}

    if (nlh_send)
    {
        free(nlh_send);
    }
    
    if (nlh_recv)
    {
        free(nlh_recv);
    }
    
	return 0;
}


int ugw_netlink_recvmsg(void **data, int *recv_len)
{
	if (NULL==data || NULL==recv_len)
	{
		printf("Invalid argument\n");
		return -1;
	}
	
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg;

    if (NULL == nlh_recv)
    {
        ugw_netlink_init();
    }
    
	memset(nlh_recv, 0, NLMSG_SPACE(MAX_PAYLOAD));
    
	iov.iov_base = (void *)nlh_recv;
	iov.iov_len = MAX_PAYLOAD;
	msg.msg_name = (void *)&(nladdr);
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	
	int nread = recvmsg(sock_fd, &msg, 0);
	if(-1 == nread)
	{
		printf("netlink recvmsg failed:%s\n", strerror(errno));
		return errno;
	}

	*data = (void*)NLMSG_DATA(nlh_recv);
    *recv_len = nread - sizeof(struct nlmsghdr);
	
	return 0;
}



int ugw_netlink_sendmsg(void *data, int len)
{
    if (NLMSG_SPACE(len) > MAX_PAYLOAD)
    {
        printf("NLMSG_SPACE(len)[%d] > MAX_PAYLOAD[%d], so no sendmsg.\n", \
            NLMSG_SPACE(len), MAX_PAYLOAD);
        return -1;
    }
    
	struct msghdr msg;
	struct sockaddr_nl dest_addr;
	struct iovec iov;
    
    if (NULL == nlh_send)
    {
        ugw_netlink_init();
    }

    memset(nlh_send, 0, NLMSG_SPACE(MAX_PAYLOAD));
	memset(&dest_addr, 0, sizeof(dest_addr));
    
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = NETLINK_GROUP_ID;

    nlh_send->nlmsg_len = NLMSG_SPACE(len);
    nlh_send->nlmsg_pid = getpid();
    nlh_send->nlmsg_flags = 0;
    
    memcpy(NLMSG_DATA(nlh_send), data, len);
	
	iov.iov_base = (void *)nlh_send;
    iov.iov_len = nlh_send->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    int ret = sendmsg(sock_fd, &msg, 0);
    if(-1 == ret)
    {
        printf("netlink sendmsg failed:%s\n", strerror(errno));
		return errno;
    }
	
	return 0;
}


/**
* @brief 
* @return 
* @remark null
* @see     
* @author ÷ÏΩ≠      @date 2012/12/06
**/
inline const char* MAC_STR(const u_int8_t mac[])
{
    static char buf[sizeof(MACSTR)] = {0};
    snprintf(buf, sizeof(MACSTR), MACSTR, \
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return buf;
}


/**
* @brief 
* @return 
* @remark null
* @see     
* @author ÷ÏΩ≠      @date 2012/12/06
**/
inline const char* PRINT_APID(u_int64_t devid)
{
    static char buf[sizeof(DEVIDSTR)] = {0};
    
#if	__BYTE_ORDER == __LITTLE_ENDIAN
    snprintf(buf, sizeof(DEVIDSTR), DEVIDSTR, \
                ((unsigned char*)&devid)[7], ((unsigned char*)&devid)[6], \
                ((unsigned char*)&devid)[5], ((unsigned char*)&devid)[4], \
                ((unsigned char*)&devid)[3], ((unsigned char*)&devid)[2], \
                ((unsigned char*)&devid)[1], ((unsigned char*)&devid)[0]);

#else
    snprintf(buf, sizeof(DEVIDSTR), DEVIDSTR, \
                ((unsigned char*)&devid)[0], ((unsigned char*)&devid)[1], \
                ((unsigned char*)&devid)[2], ((unsigned char*)&devid)[3], \
                ((unsigned char*)&devid)[4], ((unsigned char*)&devid)[5], \
                ((unsigned char*)&devid)[6], ((unsigned char*)&devid)[7]);
#endif

    return buf;
}


