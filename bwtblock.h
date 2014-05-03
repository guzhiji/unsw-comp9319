
#ifndef _BWTBLOCK_H_
#define _BWTBLOCK_H_

#include "common.h"

void bwtblock_add(bwttext * t, short pl, unsigned char cstart);

void bwtblock_scan(bwttext * t);

int bwtblock_find(bwttext * t, unsigned long pos, unsigned char c, bwtblock * blk);

void bwtblock_index_build(bwttext * t);

void bwtblock_index_load(bwttext * t);

bwtblock_index * bwtblock_index_find(bwttext * t, unsigned long pos, unsigned short * islastindex);

#endif

