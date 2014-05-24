
#ifndef _PLSET_H_
#define _PLSET_H_

#include "strbuf.h"

typedef struct {
    unsigned long pos;
    strbuf * sb1;
    strbuf * sb2;
} pline;

typedef struct {
    pline * arr;
    unsigned int max;
    unsigned int len;
} plset;

plset * plset_init();

int plset_contains(plset * s, unsigned long pos);

void plset_put(plset * s, unsigned long pos, strbuf * sb1, strbuf * sb2);

void plset_sort(plset * s);

void plset_print(plset * s, FILE * fout);

void plset_free(plset * s);

#endif

