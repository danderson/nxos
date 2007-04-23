/* Various utility functions.
 *
 * Most of these functions are defined in the libc. However, pulling
 * the libc in is quite a big hit on kernel size in some cases.
 */

#include "mytypes.h"

void memcpy(U8 *dest, const U8 *src, U32 len) {
  while (len--) {
    *dest++ = *src++;
  }
}

void memset(U8 *dest, const U8 val, U32 len) {
  while (len--) {
    *dest++ = val;
  }
}
