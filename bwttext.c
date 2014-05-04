
#include "bwttext.h"

#include "chartable.h"
#include "bwtblock.h"
#include <stdlib.h>

int _cmp_char(const void * c1, const void * c2) {
    //return (int) * (unsigned char *) c1 - (int) * (unsigned char *) c2;
    return (int) ((character *) c1)->c - ((character *) c2)->c;
}

void bwttext_scan(bwttext * t) {

    int c;
    unsigned long pos, sbefore, tsbefore;
    unsigned char cur_c;
    character * chobj, * cur_ch;

    chartable_inithash(t);

    bwtblock_scan(t);

    // sort characters lexicographically
    qsort(t->char_table, t->char_num, sizeof(character), _cmp_char);

    chartable_inithash(t);// set all null

    // calculate smaller symbols using freq 
    // to generate data for the C[] table
    c = 0; // count for boudndary
    sbefore = 0;
    cur_ch = t->char_table; // the smallest
    while (c++ < t->char_num) {
        // re-hash
        t->char_hash[(unsigned int) cur_ch->c] = cur_ch;
        // calculate smaller symbols
        tsbefore = sbefore;
        sbefore += cur_ch->ss; // accumulate freq
        cur_ch->ss = tsbefore; // smaller symbols
        cur_ch++; // a larger char
    }

}

void bwttext_index_write(bwttext * t) {

    // position of the char table
    // position of the bwtblock indices
    fseek(t->ifp, sizeof(unsigned long) * 2, SEEK_SET);
    // occ table/bwt blocks
    bwttext_scan(t);// write blocks
    // indices for the occ table
    bwtblock_index_build(t);// write indices for blokcs
    // char table
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

    t->char_num = 0;

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

