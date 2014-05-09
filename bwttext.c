
#include "bwttext.h"

#include "chartable.h"
#include "occtable.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * INDEX FILE STRUCTURE:
 * index info:
 * - file size
 * - block num
 * - block width
 * - freq char num
 * - char num
 *
 * occ table:
 * - occ for freq chars
 * - occ for less freq chars
 * = unsigned long * char_num * block_num
 *
 * char table:
 * - character(ss, sorted by char), character * char_num
 */
void bwttext_index_write(bwttext * t) {
    unsigned char num;

    // INDEX INFO:

    // compute:
    chartable_compute_charfreq(t);

    fseek(t->ifp, 0, SEEK_SET);

    // file size:
    fwrite(&t->file_size, sizeof(unsigned long), 1, t->ifp);

    // block num:
    fwrite(&t->block_num, sizeof(unsigned long), 1, t->ifp);

    // block width:
    fwrite(&t->block_width, sizeof(unsigned long), 1, t->ifp);

    // freq char num:
    fwrite(&t->char_freq_num, sizeof(short), 1, t->ifp);

    // char num:
    // maximum 256, fit it in 1 byte
    // since char_num can't be 0
    num = (unsigned char) (t->char_num - 1);
    fwrite(&num, sizeof(unsigned char), 1, t->ifp);

    // OCC TABLE:

    occtable_init(t, 0);
    occtable_generate(t);//requires freq

    // CHARTABLE:

    chartable_compute_ss(t);//consumes freq
    chartable_save(t);//requires ss

}

void bwttext_index_load(bwttext * t) {
    unsigned char num;

    fseek(t->ifp, 0, SEEK_SET);
    // file size:
    fread(&t->file_size, sizeof(unsigned long), 1, t->ifp);

    // block num:
    fread(&t->block_num, sizeof(unsigned long), 1, t->ifp);

    // block width:
    fread(&t->block_width, sizeof(unsigned long), 1, t->ifp);

    // freq char num:
    fread(&t->char_freq_num, sizeof(short), 1, t->ifp);

    // char num:
    fread(&num, sizeof(unsigned char), 1, t->ifp);
    t->char_num = 1 + num;

    occtable_init(t, 1);
    chartable_load(t);//requires occ_infreq_pos, etc.
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
        if (t->fp != NULL) {
            fclose(t->fp);
            t->fp = NULL;
        }
        if (t->ifp != NULL) {
            fclose(t->ifp);
            t->ifp = NULL;
        }
        occtable_free(t);
        free(t);
    }
}

