#ifndef __NXOS_RADAR_H__
#define __NXOS_RADAR_H__

#include "mytypes.h"

void radar_init(U8 sensor);
bool radar_info(U8 sensor);
void radar_test(U8 sensor);

#endif
