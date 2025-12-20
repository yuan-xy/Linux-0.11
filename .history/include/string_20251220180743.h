#ifndef _STRING_H_
#define _STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

extern char * strerror(int errno);

/*
 * This header originally tried to provide lots of "extern inline" string
 * routines. Modern GCC (e.g. GCC 15) has different inline semantics and
 * will warn/error if inline functions are declared but not defined in the
 * same translation unit.
 *
 * For toolchain compatibility, we provide normal extern prototypes here and
 * let the real implementations be linked from lib/string.c (and friends).
 *
 *      (C) 1991 Linus Torvalds
 */

/* ---- memory operations ---- */
extern void * memset(void * s, int c, int count);
extern void * memcpy(void * dest, const void * src, int n);
extern void * memmove(void * dest, const void * src, int n);
extern void * memchr(const void * cs, char c, int count);
extern int    memcmp(const void * cs, const void * ct, int count);

/* ---- basic string operations ---- */
extern int    strlen(const char * s);

extern char * strcpy(char * dest, const char * src);
extern char * strncpy(char * dest, const char * src, int n);

extern char * strcat(char * dest, const char * src);

extern int    strcmp(const char * cs, const char * ct);
extern int    strncmp(const char * cs, const char * ct, int n);

extern char * strchr(const char * s, char c);
extern char * strrchr(const char * s, char c);

extern int    strspn(const char * cs, const char * ct);
extern int    strcspn(const char * cs, const char * ct);
extern char * strpbrk(const char * cs, const char * ct);
extern char * strstr(const char * cs, const char * ct);

/* strtok() keeps legacy helper symbol */
extern char * ___strtok;
extern char * strtok(char * s, const char * ct);

#endif /* _STRING_H_ */