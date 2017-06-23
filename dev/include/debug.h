/*! \file
 *  \brief Debugging macros
 */
#ifndef _DEBUG_H
#define _DEBUG_H
#include <stdio.h>

extern int debug;

#ifdef DEBUG
/*! \brief Standard debug macro  */
#define dbg() if(debug) { fprintf(stderr, "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__); }

/*! \brief Debug print macro */
#define dprint 	if(debug) { fprintf(stderr, "%s %s %d: ", __FILE__, __FUNCTION__, __LINE__); } _dprint

#define malloc dmalloc
#else 
#define dbg() {}
#define dprint {}
#endif

int _dprint(char *fmt, ...);

void *dmalloc(size_t size);

#endif /* _DEBUG_H */
