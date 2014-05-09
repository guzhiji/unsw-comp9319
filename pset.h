
#ifndef _PSET_H_
#define _PSET_H_

typedef struct {
    unsigned long * arr;
    unsigned int max;
    unsigned int len;
} pset;

pset * pset_init();

int pset_contains(pset * s, unsigned long pos);

void pset_put(pset * s, unsigned long pos);

void pset_free(pset * s);

#endif

