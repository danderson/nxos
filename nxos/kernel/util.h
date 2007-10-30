#ifndef __NXOS_UTIL_H__
#define __NXOS_UTIL_H__

#include "mytypes.h"

#define MIN(x, y) ((x) < (y) ? (x): (y))
#define MAX(x, y) ((x) > (y) ? (x): (y))

void memcpy(U8 *dest, const U8 *src, U32 len);
void memset(U8 *dest, const U8 val, U32 len);
U32 strlen(const char *str);

U8 strncmp(const char *a, const char *b, U32 n);
U8 strcmp(const char *a, const char *b);

#endif
