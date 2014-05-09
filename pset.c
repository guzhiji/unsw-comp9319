
#include "pset.h"

#include <stdlib.h>

pset * pset_init() {
    pset * s = (pset *) malloc(sizeof(pset));
    s->arr = (unsigned long *) malloc(sizeof(unsigned long) * 32);
    s->max = 32;
    s->len = 0;
    return s;
}

int pset_contains(pset * s, unsigned long pos) {
    unsigned long i, l;
    l = s->len;
    for (i = 0; i < l; i++)
        if (s->arr[i] == pos)
            return 1;
    return 0;
}

void pset_put(pset * s, unsigned long pos) {

    //if (pset_contains(s, pos)) return;

    if (s->len == s->max) {

        unsigned long * ns = (unsigned long *) realloc(s->arr, sizeof(unsigned long) * (s->max + 32));

        if (ns == NULL) return;

        s->max += 32;
        s->arr = ns;
    }

    s->arr[s->len++] = pos;

}

void pset_free(pset * s) {
    free(s->arr);
    free(s);
}

