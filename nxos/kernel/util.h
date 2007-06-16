#ifndef __NXOS_UTIL_H__
#define __NXOS_UTIL_H__

#include "mytypes.h"

#define MIN(x, y) ((x) < (y) ? (x): (y))
#define MAX(x, y) ((x) > (y) ? (x): (y))

void memcpy(U8 *dest, const U8 *src, U32 len);
void memset(U8 *dest, const U8 val, U32 len);

#endif
