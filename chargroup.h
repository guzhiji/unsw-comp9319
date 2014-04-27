
/**
 * chargroup: manages memory usage of character groups
 */

#ifndef _CHARGROUP_H_
#define _CHARGROUP_H_

#include "bwttext.h"

#define CHARGROUP_NUM_THRESHOLD 196608
#define CHARGROUP_NUM_KEEP 196608
//98304

#define CHARGROUP_LIST_SIZE_INIT 10240
#define CHARGROUP_LIST_SIZE_STEP 10240

#define CHARGROUP_LIST_POS_SIZE_INIT 10
#define CHARGROUP_LIST_POS_SIZE_STEP 10

/**
 * get a char group list.
 *
 * if it's not available in the memory,
 * reads it from the index file.
 */
chargroup_list * chargroup_list_get(bwttext * t, unsigned char c);

/**
 * add a char group.
 *
 * it the amount in memory grows over the threshold,
 * writes them into the index file and release them.
 */
void chargroup_list_add(bwttext * t, unsigned char c, chargroup * cg);

/**
 * initialize a chargroup list
 */
chargroup_list * chargroup_list_init();

void chargroup_list_savereleaseall(bwttext * t);

#endif
