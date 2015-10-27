#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>

extern "C" {


#include <lua.h>
#include <lauxlib.h> 

static int l_dumb(lua_State *L) {
	return 0;
}

static int s_sock = 0;
static int init_sock() {
	if (s_sock > 0)
		return s_sock;
	s_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (s_sock <= 0) {
		return -1;
	}
	return s_sock;
}

static void close_sock() {
	if (s_sock > 0)
		close(s_sock);
	s_sock = 0;
}

static int l_interface(lua_State *L) {
	int sock = init_sock();
	if (sock <= 0) {
		lua_pushnil(L);
		lua_pushstring(L, "init_sock fail");
		return 2;
	}
	
	char buf[2048];
	struct ifconf ifconf;
	
	ifconf.ifc_buf = buf;
	ifconf.ifc_len = sizeof(buf);
	
	int ret = ioctl(sock, SIOCGIFCONF, &ifconf);
	if (ret) {
		int err = errno;
		lua_pushnil(L);
		lua_pushfstring(L, "ioctl fail %s", strerror(err));
		return 2;
	}
	
	struct ifreq *ifreq = ifconf.ifc_req;	assert(ifreq);

	lua_newtable(L);
	for (int i = 0, total = ifconf.ifc_len / sizeof(struct ifreq); i < total; i++, ifreq++) {
		lua_newtable(L);
		
		if (!ioctl(sock, SIOCGIFFLAGS, ifreq)) {
			char ipaddr[24];
			struct sockaddr_in *in_addr = (struct sockaddr_in *)&ifreq->ifr_addr;
			if(inet_ntop(AF_INET, &in_addr->sin_addr, ipaddr, sizeof(ipaddr))) {
				lua_pushstring(L, ipaddr);
				lua_setfield(L, -2, "ip");
			}
		}
		
		if (!ioctl(sock, SIOCGIFBRDADDR, ifreq)) {
			char ipaddr[24];
			struct sockaddr_in *in_addr = (struct sockaddr_in *)&ifreq->ifr_addr;
			if(inet_ntop(AF_INET, &in_addr->sin_addr, ipaddr, sizeof(ipaddr))) {
				lua_pushstring(L, ipaddr);
				lua_setfield(L, -2, "broadip");
			}
		}
		
		if (!ioctl(sock, SIOCGIFNETMASK, ifreq)) { 
			struct sockaddr_in *netmask = (struct sockaddr_in *)&(ifreq->ifr_netmask);
			const char *mask = inet_ntoa(netmask->sin_addr);
			if (mask) {
				lua_pushstring(L, mask);
				lua_setfield(L, -2, "mask");
			} 
		}
		if (!ioctl(sock, SIOCGIFHWADDR, ifreq)) {
			char mac[24] = {0};
			unsigned char *adr =(unsigned char *)ifreq->ifr_hwaddr.sa_data;
			sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", adr[0], adr[1], adr[2], adr[3], adr[4], adr[5]);
			lua_pushstring(L, mac);
			lua_setfield(L, -2, "mac"); 
		}
		
		lua_setfield(L, -2, ifreq->ifr_name);
	}
	
	return 1;
}

static luaL_Reg reg[] = {
	{ "ifc", 		l_interface },
	{ NULL, 		NULL }
};
	
LUALIB_API int luaopen_netp(lua_State *L) {
	luaL_register(L, "netp", reg);
	return 1;
}
}