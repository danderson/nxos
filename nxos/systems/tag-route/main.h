/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_TAG_ROUTE_MAIN_H__
#define __NXOS_TAG_ROUTE_MAIN_H__

#include "base/types.h"

#define ROUTE_FILE "tag.data"

void record(char *filename);
void replay(char *filename);
void usb_recv(void);

#endif /* __NXOS_TAG_ROUTE_MAIN_H__ */

