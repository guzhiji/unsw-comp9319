
#ifndef _BWTTEXT_H_
#define _BWTTEXT_H_

#include "common.h"

bwttext * bwttext_init(char * bwtfile, char * indexfile, unsigned char special_char, int buildindex);

void bwttext_free(bwttext * t);

#endif

