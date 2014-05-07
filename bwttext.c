
#include "bwttext.h"

#include "chartable.h"
#include "bwtblock.h"
#include <stdlib.h>

void bwttext_index_write(bwttext * t) {

    // INDEX FILE STRUCTURE:

    // position of the char table
    // position of the bwtblock indices

    // bwt blocks
    t->char_num = 0;
    chartable_inithash(t);
    bwtblock_scan(t);

    // occ table
    t->char_num = 0;
    chartable_inithash(t);
    bwtblock_occ_compute(t);

    // indices for blokcs
    bwtblock_index_build(t);

    // char table
    chartable_ss_compute(t);
    chartable_save(t);

}

void bwttext_index_load(bwttext * t) {

    bwtblock_index_load(t);
    chartable_load(t);

}

bwttext * bwttext_init(char * bwtfile, char * indexfile, int forceindex) {

    bwttext * t = (bwttext *) malloc(sizeof(bwttext));

    t->fp = fopen(bwtfile, "rb");
    if (t->fp == NULL) {
        bwttext_free(t);
        exit(1);
    }
    fread(&t->end, sizeof(unsigned long), 1, t->fp);

    t->ifp = NULL;
    if (!forceindex) { // try to read
        t->ifp = fopen(indexfile, "rb");
        if (t->ifp != NULL)
            bwttext_index_load(t);
    }
    if (t->ifp == NULL) { // force to index or fail to read
        t->ifp = fopen(indexfile, "w+b");
        if (t->ifp == NULL) {
            bwttext_free(t);
            exit(1);
        }
        bwttext_index_write(t);
    }

    return t;
}

void bwttext_free(bwttext * t) {
    if (t != NULL) {
        if (t->fp != NULL)
            fclose(t->fp);
        if (t->ifp != NULL)
            fclose(t->ifp);
        free(t);
    }
}

