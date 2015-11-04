#pragma once

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include <lua.h>
#include <lauxlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#if LUA_VERSION_NUM < 502
#define LUA_OK	0
#define lua_rawlen lua_objlen
/* lua_...uservalue: Something very different, but it should get the job done */
#define lua_getuservalue lua_getfenv
#define lua_setuservalue lua_setfenv
#define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#define luaL_setfuncs(L,l,n) (assert(n==0), luaL_register(L,NULL,l))
#define lua_resume(L,F,n) lua_resume(L,n)
#endif


#if LUA_VERSION_NUM < 502 
# define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#endif 

#if LUA_VERSION_NUM > 501
/*
** Lua 5.2
*/
#define lua_strlen lua_rawlen
/* luaL_typerror always used with arg at ndx == NULL */
#define luaL_typerror(L,ndx,str) luaL_error(L,"bad argument %d (%s expected, got nil)",ndx,str)
/* luaL_register used once, so below expansion is OK for this case */
#define luaL_register(L,name,reg) lua_newtable(L);luaL_setfuncs(L,reg,0)
/* luaL_openlib always used with name == NULL */
#define luaL_openlib(L,name,reg,nup) luaL_setfuncs(L,reg,nup)

#if LUA_VERSION_NUM > 502
/*
** Lua 5.3
*/
#define luaL_checkint(L,n)  ((int)luaL_checkinteger(L, (n)))
#endif
#endif

//TODO: exit directly
#define oom_check(p) do { \
	if (!(p)) { \
		fprintf(stderr, "oom"); \
		abort(); \
	} \
} while (0)

static __inline void *se_malloc(size_t size)
{
	void *p;

	p = malloc(size);
	oom_check(p);
	return p;
}

static __inline void se_free(void *p)
{
	free(p);
}

#define se_new(type)		(type *)se_malloc(sizeof(type))
#define se_new_n(type, n)	(type *)se_malloc(sizeof(type) * (n))

#define debug_checkpoint()	printf("checkpoint at %s:%d\n", __FILE__, __LINE__)