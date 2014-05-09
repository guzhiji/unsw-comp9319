
#ifndef _OCCTABLE_H_
#define _OCCTABLE_H_

#include "common.h"

void occtable_init(bwttext * t, int load);

void occtable_free(bwttext * t);

unsigned long occtable_offset(bwttext * t, character * ch, unsigned long pos);

unsigned long bwtblock_offset(bwttext * t, unsigned long pos);

void occtable_generate(bwttext * t);

#endif

