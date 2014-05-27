
#ifndef _OCC_H_
#define _OCC_H_

#include "bwttext.h"

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos);

unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ);

#endif

