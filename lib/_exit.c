/*
 *  linux/lib/_exit.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <unistd.h>

void _exit(int exit_code)
{
	__asm__ __volatile__ ("int $0x80"::"a" (__NR_exit),"b" (exit_code));
}
