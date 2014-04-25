
#include "bwttext.h"
#include "bwtsearch.h"
#include "bwtindex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    bwttext * t;
    fpos_range * r;
    char * p = "Pride";
/*
h 167 169
c 137 139
a 108 109
e 149 150
p 211 212
found
y 252 254
r 237 239
r 233 235
found
 */
/*
    t = bwttext_init("../tests/bwtsearch/tiny.bwt", "../tiny.idx");
*/
/*
    t = bwttext_init("../tests/bwtsearch/japan.bwt", "../japan.idx");
*/
/*
    t = bwttext_init("../tests/bwtsearch/sherlock.bwt", "../sherlock.idx");
*/
/*
    t = bwttext_init("../tests/bwtsearch/pride.bwt", "../pride.idx");
*/
    t = bwttext_init("../tests/bwtsearch/gcc.bwt", "../gcc.idx", 1);

    //{
    bwtindex_chartable_presave(t);
    bwttext_read(t);
    bwtindex_chartable_save(t);
    // OR
/*
    bwtindex_chartable_load(t);
*/
    //}
    //dump_chartable(t);
    dump_pos(t);
    //decode_backword(t);
/*

    r = search_range(t, p, strlen(p));
    // TODO reverse
    if (r != NULL)
        printf("f-l=%lu-%lu", r->first, r->last);
    free(r);
*/

    //occ for 3
    FILE * tt=fopen("../tests/bwtsearch/gcc.bwt", "rb");
    FILE * ttt=fopen("../gcc.c.out", "w");
    int ttc;
    unsigned long pos = 0;
    fseek(tt, 4, SEEK_SET);
    while ((ttc=fgetc(tt))!=EOF) {
        if (ttc == '3' && pos >= 488261) {
            fprintf(ttt, "%lu %lu\n", pos, occ(t, (unsigned char) ttc, pos));
        }
        pos++;
    }
    fclose(ttt);
    fclose(tt);
    /*
output:
488261 780
489218 473914
696464 681160
696465 681161
709017 693713
710003 694699
710004 694700
711092 695788

but it should be:
488261 780
489218 781
696464 782
696465 783
709017 784
710003 785
710004 786
711092 787
     */
    
    bwttext_free(t);
    return 0;
}
