
#include "bwttext.h"

#include "chartable.h"
#include "bwtblock.h"
#include <stdlib.h>

unsigned long bwttext_size(FILE * fp) {
    unsigned char cb[1024];
    unsigned int r;
    unsigned long l = 0;
    fpos_t origin;

    fgetpos(fp, &origin);
    fseek(fp, 4, SEEK_SET);
    do {
        r = fread(&cb, sizeof(unsigned char), 1024, fp);
        l += r;
    } while (r > 0);
    fsetpos(fp, &origin);

    return l;
}

/**
 * structure:
 * - position of the char table
 * - position of the bwtblock index
 * - bwt blocks
 * - occ table
 * - bwt block index
 * - char table
 */
void bwttext_index_write(bwttext * t) {

    // initialize
    t->char_num = 0;
    t->file_size = bwttext_size(t->fp);
    chartable_inithash(t);

    // bwt blocks
    bwtblock_prepare(t); // a rough scan and prepare blocks
    // occ table
    bwtblock_occ_compute(t);

    // bwt block index
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
        fprintf(stderr, "error: could not read the BWT file\n");
        exit(1);
    }
    // end position of the BWT text
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
            fprintf(stderr, "error: could not write the index file\n");
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

