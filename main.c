
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dump_occ(bwttext * t, unsigned char c, char * outfile) {
    FILE * ttt = fopen(outfile, "w");
    int ttc;
    unsigned long pos = 0;
    fseek(t->fp, 4, SEEK_SET);
    while ((ttc = fgetc(t->fp)) != EOF) {
        fprintf(ttt, "%lu %lu\n", pos, occ(t, c, pos));
        pos++;
    }
    fclose(ttt);
}

int main(int argc, char **argv) {
    bwttext * t;
    fpos_range * r;
    unsigned long i;
    char * p = "ea";
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
            t = bwttext_init("../tests/bwtsearch/tiny.bwt", "../tiny.idx", 1);
     */
    t = bwttext_init("../tests/bwtsearch/japan.bwt", "../japan.idx", 1);
    /*
                t = bwttext_init("../tests/bwtsearch/sherlock.bwt", "../sherlock.idx", 1);
     */
    /*
            t = bwttext_init("../tests/bwtsearch/pride.bwt", "../pride.idx", 1);
     */
    /*
            t = bwttext_init("../tests/bwtsearch/gcc.bwt", "../gcc.idx", 1);
     */

    /*
            dump_chartable(t);
     */

    decode_backward(t);

    /*
        r = search_fpos_range(t, p, strlen(p));
        // TODO reverse
        if (r != NULL) {
            printf("found between f-l=%lu-%lu\n", r->first, r->last);
            for (i = r->first; i <= r->last; i++) {
                printf("\n%lu:\n", i);
                decode_backward_until(t, i, '\n');
                decode_forward_until(t, i, '\n');
            }
        } else {
            printf("not found\n");
        }
        free(r);
     */
    /*
        dump_occ(t, 'e', "../tiny.c.out");
     */
    /*
            char fn[50];
            for (i = 0; i < 256; i++) {
                sprintf(fn, "../occtest/japan.c.%lu.out", i);
                dump_occ(t, (unsigned char) i, fn);
            }
     */

    bwttext_free(t);
    return 0;
}
