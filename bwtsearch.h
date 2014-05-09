
/**
 * bwtsearch: implements BWT backward & forward search
 */

#ifndef _BWTSEARCH_H_
#define _BWTSEARCH_H_

#include "common.h"

typedef struct {
    unsigned long first;
    unsigned long last;
} fpos_range;

/**
 * @param t     ref to processed bwttext
 * @param p     ref to the queried pattern
 * @param l     length of the pattern
 * @return      range in the first column of the BWT rotation matrix
 */
fpos_range * search_fpos_range(bwttext * t, unsigned char * p, unsigned int l);

void search(bwttext * t, unsigned char * p, unsigned int l);

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos);

void decode_backward(bwttext * t, FILE * fout);

void decode_backward_rev(bwttext * t, FILE * fout);

#endif
