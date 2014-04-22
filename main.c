
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE * fp = fopen("tests/tiny.bwt", "rb");
    FILE * ifp = fopen("tiny.idx", "wrb");
    bwttext * t = (bwttext *) malloc(sizeof(bwttext));
    fpos_range * r;
    unsigned char * p = "abcd";

    t->fp = fp;
    t->ifp = ifp;
    bwttext_read(t);
    chargroup_list_savereleaseall(t);
    // sort by freq
    r = search_range(t, p, strlen(p));
    bwtindex_free(t);
    fclose(fp);
    fclose(ifp);
    printf("f-l=%lu-%lu", r->first, r->last);
    free(r);

    return 0;
}
