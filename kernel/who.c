#include <string.h>
#include <errno.h>
#include <asm/segment.h>

char msg[24]; //最多23个字符，外带一个回车符号

int 
sys_iam(const char* name)
{
	int len = 0;
	int tmp[30];
	int i = 0;
	for ( len = 0; len < 30; len++) 
	{
		tmp[len] = get_fs_byte(name+len);		
		if (tmp[len] == '\0' || tmp[len] == '\n') 
		{
			break;		
		}
	}
	if (len >= 24) 
	{
		return -EINVAL;		
	}
	for(i=0;i <= len;i++)
	{
		msg[i] = tmp[i];
	}
	return	len;
}

int
sys_whoami(char* name,unsigned int size)
{
	int len = 0;
	int i = 0;
	for(; msg[len] != '\0'; len++ );
	if (len > size) {
		return -EINVAL;	
	}
	for (i = 0; i < size; ++i) {
		put_fs_byte(msg[i],name+i);	
		if (msg[i] == '\0') {
			break;	
		}
	}
	return i;
}
