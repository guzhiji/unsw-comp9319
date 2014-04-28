
#ifndef _BWTTEXT_H_
#define _BWTTEXT_H_

#include "common.h"

bwttext * bwttext_init(char * bwtfile, char * indexfile, int buildindex);

//void bwttext_read(bwttext * t);

void bwttext_free(bwttext * t);

#endif

