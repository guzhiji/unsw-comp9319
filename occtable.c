
#include "occtable.h"

#include <stdio.h>
#include <stdlib.h>

void occtable_init(bwttext * t, int load) {

    unsigned long n;

    if (t->char_freq_num > 0) {

        // excluding the snapshot after the last block
        n = t->char_freq_num * (t->block_num - 1);
        t->occ_freq = (unsigned long *) malloc(sizeof(unsigned long) * n);

        if (load)
            fread(t->occ_freq, sizeof(unsigned long), n, t->ifp);

    } else {
        n = 0;
        t->occ_freq = NULL;
    }

    t->occ_infreq_pos = OCCTABLE_START + n;

}

void occtable_free(bwttext * t) {
    if (t->occ_freq != NULL) {
        free(t->occ_freq);
        t->occ_freq = NULL;
    }
}

/**
 * @param t     BWT text object
 * @param ch    which character
 * @param pos   position in BWT text
 * @return      occ offset in the occtable
 */
unsigned long occtable_offset(bwttext * t, character * ch, unsigned long pos) {
    // char start pos + snapshot offset
    return ch->i * t->block_num + (pos + 1) / t->block_width - 1;
}

/**
 * @param t     BWT text object
 * @param pos   position in BWT text
 * @return      char offset of the block where the position locates
 */
unsigned long bwtblock_offset(bwttext * t, unsigned long pos) {
    // number of blocks * number of chars per block - 1
    return (pos + 1) / t->block_width * t->block_width - 1;
}

void occtable_generate(bwttext * t) {

    character * ch;
    unsigned long blocks, n, pos, offset;
    int ic;

    // clear frequencies so as to count occ
    for (ic = 0; ic < t->char_num; ic++)
        t->char_table[ic].ss = 0;

    pos = 0; // current position in the BWT file
    n = 0; // number of chars in the current block
    blocks = 1; // block count

    fseek(t->fp, 4, SEEK_SET);
    while ((ic = fgetc(t->fp)) != EOF) {

        ch = t->char_hash[ic];
        if (ch == NULL) {
            fprintf(stderr, "occtable_write(): encounter unknown char\n");
            exit(1);
        }

        // write occ before counting the current occurance
        if (++n == t->block_width && blocks < t->block_num) {
            // a block of block_width is read
            // never count the last block

            // the next char belongs to a newer block
            n = 0;
            blocks++;

            // find the position where occ is stored
            offset = occtable_offset(t, ch, pos);
            if (ch->isfreq) {
                t->occ_freq[offset] = ch->ss;
                fseek(t->ifp, OCCTABLE_START + offset, SEEK_SET);
            } else {
                fseek(t->ifp, t->occ_infreq_pos + offset, SEEK_SET);
            }
            fwrite(&ch->ss, sizeof(unsigned long), 1, t->ifp);

        }

        ch->ss++;
        pos++;
    }

    // char frequencies recovered

}

