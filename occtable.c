
#include "occtable.h"

#include <stdio.h>
#include <stdlib.h>

void occtable_init(bwttext * t, int load) {

    unsigned long n;

    if (t->char_freq_num > 0) {

        // no snapshot after the last block
        n = t->char_freq_num * (t->block_num - 1);
        t->occ_freq = (unsigned long *) malloc(sizeof (unsigned long) * n);

        if (load) {
            // currently not necessary as it is called after ftell()
            // fseek(t->ifp, t->occ_freq_pos, SEEK_SET);
            fread(t->occ_freq, sizeof (unsigned long), n, t->ifp);
        }

    } else {
        n = 0;
        t->occ_freq = NULL;
    }

    t->occ_infreq_pos = t->occ_freq_pos + n * sizeof (unsigned long);

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
 * @param pos   position in BWT text, it MUST come AFTER the first snapshot
 * @return      occ offset in the occtable
 */
unsigned long occtable_offset(bwttext * t, character * ch, unsigned long pos) {
    // char start pos + snapshot offset
    return ch->i * (t->block_num - 1) + pos / t->block_width - 1;
}

void occtable_generate(bwttext * t) {

    character * ch, * tch;
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

        if (++n == 1 && blocks > 1) {
            // at the first char after the first block
            // create an occ snapshot for all chars for the preceding block
            for (ic = 0; ic < t->char_num; ic++) {
                tch = &t->char_table[ic];
                // find the position where occ is stored
                offset = occtable_offset(t, tch, pos);
                if (tch->isfreq) {
                    t->occ_freq[offset] = tch->ss;
                    fseek(t->ifp, t->occ_freq_pos + offset * sizeof (unsigned long), SEEK_SET);
                } else {
                    fseek(t->ifp, t->occ_infreq_pos + offset * sizeof (unsigned long), SEEK_SET);
                }
                fwrite(&tch->ss, sizeof (unsigned long), 1, t->ifp);
            }
        } else if (n == t->block_width) {
            // a block of block_width is read
            // the next char belongs to a newer block
            n = 0;
            blocks++;
        }

        ch->ss++;
        pos++;
    }

    // char frequencies recovered

}
