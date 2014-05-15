
#ifndef _BWTBLOCK_H_
#define	_BWTBLOCK_H_

#include "common.h"

typedef struct {
    unsigned long occ;
    unsigned long offset;
} bwtblock_start;

typedef struct {
    unsigned char * data;
    unsigned int size;
} bwtblock_cached_data;

typedef struct {
    unsigned long offset;
    unsigned char data[1024];
    unsigned int visits;
} bwtblock_cached_block;

unsigned long bwtblock_offset(bwttext * t, unsigned long pos);

void bwtblock_start_lookup(bwttext * t, character * ch, unsigned long occ, bwtblock_start * start);

unsigned long bwtblock_occ(bwttext * t, bwtblock_start * start, unsigned char c, unsigned long pos_until);

unsigned long bwtblock_occ_position(bwttext * t, bwtblock_start * start, unsigned char c, unsigned long occ_until);

#endif
