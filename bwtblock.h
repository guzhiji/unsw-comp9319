
#ifndef _BWTBLOCK_H_
#define _BWTBLOCK_H_

#include "common.h"

void bwtblock_addchar(bwttext * t, character * c, unsigned long p);

void bwtblock_buildindex(bwttext * t);

void bwtblock_loadindex(bwttext * t);

#endif

