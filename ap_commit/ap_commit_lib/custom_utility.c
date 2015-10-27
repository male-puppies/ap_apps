#include <string.h>
#include "custom_utility.h"

void strupper(char *str)
{
	if (NULL == str)
	{
		return ;
	}
	
	int str_len = strlen(str);
	int i;
	for (i = 0; i < str_len; i++)
	{
		if (!isupper(str[i]))
		{
			str[i] = toupper(str[i]);
		}
	}
	
	return ;
}

