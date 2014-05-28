
#ifndef _BWTTEXT_H_
#define _BWTTEXT_H_

#include "common.h"

bwttext * bwttext_init(char * bwtfile, unsigned char special_char);

void bwttext_free(bwttext * t);

#endif

