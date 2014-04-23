
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    bwttext * t;
    fpos_range * r;
    unsigned char * p = "peach";

    //t = bwttext_init("../tests/bwtsearch/tiny.bwt", "../tiny.idx");
    t = bwttext_init("../tests/bwtsearch/gcc.bwt", "../gcc.idx");

    //{
    bwtindex_chartable_presave(t);
    bwttext_read(t);
    bwtindex_chartable_save(t);
    // OR
    //bwtindex_chartable_load(t);
    //}

    r = search_range(t, p, strlen(p));
    // reverse
    bwttext_free(t);
    if (r != NULL)
        printf("f-l=%lu-%lu", r->first, r->last);
    free(r);

    return 0;
}
