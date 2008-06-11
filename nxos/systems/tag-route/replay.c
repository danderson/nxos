/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/types.h"
#include "base/display.h"
#include "base/lib/rcmd/rcmd.h"

#include "main.h"

void replay(char *filename) {
  nx_display_clear();
  nx_rcmd_parse(filename);
}

