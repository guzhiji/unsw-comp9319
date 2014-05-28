
#ifndef _CHARTABLE_H_
#define _CHARTABLE_H_

#include "common.h"

void chartable_inithash(bwttext * t);

void chartable_compute_charfreq(bwttext * t);

void chartable_compute_ss(bwttext * t, unsigned char special_char);

#endif

