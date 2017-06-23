/*! \file Debug functions
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "debug.h"

#ifdef DEBUG
int debug = 1;
#else
int debug = 0;
#endif /* DEBUG */


/*! \brief Debug print
 */
int _dprint(char *fmt, ...) {
 int res=0;
 if (debug) {
 	va_list list;
	va_start(list,fmt); 
 	res=vfprintf(stderr, fmt, list); 
	va_end(list);
 }
 return res;
}

#ifdef malloc 
#undef malloc
#include <stdlib.h>

void *dmalloc(size_t size){
	void *res = malloc(size);
	if (res==0) { dprint("malloc: %s\n", strerror(errno)); }
	return res;
}
#endif
