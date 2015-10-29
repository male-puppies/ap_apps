#ifndef _CUSTOM_UTILITY_H_
#define _CUSTOM_UTILITY_H_
#include <stdio.h>

static char *safe_strncpy(char *dst, const char *src, const size_t len)
{
	assert(dst);
	assert(src);
	if (strlen(src) >= len)
	{
		strncpy(dst, src, len - 1);
		dst[len - 1] = '\0';
	}
	else
	{
		strncpy(dst, src, strlen(src));
		dst[strlen(src)] = '\0';
	}
	return NULL;
}

void strupper(char *str);

#endif

