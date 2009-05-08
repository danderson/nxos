/* Copyright (c) 2009 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

/* Assertion checking example.
 *
 * NxOS provides a simplistic assertion checking facility, which can
 * be used to crash the system gracefully, printing a debug dump to
 * the screen. Use this functionality to enforce "can't happen"
 * conditions.
 */

#include "base/assert.h"

void main() {
  /* Let's test basic mathematics. If this fails, the CPU is so badly
   * damaged that it's a miracle we booted at all :-).
   */
  U32 a = 2;
  U32 b = 2;
  a += b;

  /* Basic assert, which prints the failing assertion, a CPU state
   * dump, and "assertion failed" to the screen before crashing.
   */
  NX_ASSERT(a == 4);

  a += b;

  /* Same as NX_ASSERT, except that "assertion failed" is replaced
   * with the (short) message of your choosing.
   */
  NX_ASSERT_MSG(a == 6, "Universe broken");

  a += a;

  /* If you have a complex test that cannot fit in NX_ASSERT, you can
   * use an explicit if, and use NX_FAIL to crash unconditionally
   * (essentially, NX_FAIL is an always-failing NX_ASSERT).
   */
  if (a != 12) {
    NX_FAIL("Universe broken");
  }
}
