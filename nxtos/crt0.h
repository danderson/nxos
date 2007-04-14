/*
 * Helper functions installed by the bootstrap code.
 */

#ifndef __ESTORM_CRT0_H__
#define __ESTORM_CRT0_H__

#include "types.h"

extern void interrupts_disable();
extern void interrupts_enable();

extern void memcpy(U8 *dest, U8 *src, U32 len);
extern void memset(U8 *dest, U8 val, U32 len);

#endif /* __ESTORM_CRT0_H__ */
