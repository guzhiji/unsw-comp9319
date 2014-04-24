
#include "bwttext.h"
#include "bwtsearch.h"
#include "bwtindex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    bwttext * t;
    fpos_range * r;
    char * p = "peach";
/*
h 167 169
c 137 139
a 108 109
e 149 150
p 211 212
found
 */
    t = bwttext_init("../tests/bwtsearch/tiny.bwt", "../tiny.idx");
    //t = bwttext_init("../tests/bwtsearch/gcc.bwt", "../gcc.idx");

    //{
    bwtindex_chartable_presave(t);
    bwttext_read(t);
    bwtindex_chartable_save(t);
    // OR
    //bwtindex_chartable_load(t);
    //}
    //decode(t);

    r = search_range(t, p, strlen(p));
    // reverse
    bwttext_free(t);
    if (r != NULL)
        printf("f-l=%lu-%lu", r->first, r->last);
    free(r);

    return 0;
}
