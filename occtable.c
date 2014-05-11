
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

/**
 * @param t     BWT text object
 * @param pos   position in BWT text
 * @return      char offset of the block where the position locates
 */
unsigned long bwtblock_offset(bwttext * t, unsigned long pos) {
    // number of blocks * number of chars per block - 1 + 1
    return pos / t->block_width * t->block_width;
}

/**
 * look up block offset through occ snapshots
 *
 * @param s     snapshots (occ values)
 * @param l     number of snapshots
 * @param occ   the target occ value
 */
unsigned int _blkoffset_lookup(unsigned long * s, unsigned int l, unsigned long occ) {
    unsigned int i;

    // if it isn't after snapshot i,
    // it is snapshot i-1;
    // the block after snapshot i-1 is block i;
    // even for the one before snapshot 0,
    // it is block 0
    for (i = 0; i < l; i++) {
        if (occ < s[i])
            return i;
        // A problem arises because of change of block width 
        // after finding that file size isn't divideable by 
        // block number in compute_mem_maxchars() of chartable.c.
        // Consequently, there can be some slots not being used here,
        // so more code is added to detect the issue.
        if (i > 0 && s[i - 1] > 0 && s[i] == 0)
            return i - 1;
    }
    // after all snapshots,
    // or after the last snapshot, snapshot l-1,
    // it is block l (since blocks=snapshots+1)
    return l;

}

void bwtblock_offset_lookup(bwttext * t, character * ch, unsigned long occ, unsigned long * blk_offset, unsigned long * occ_start) {
    unsigned long l, offset, blk;

    l = t->block_num - 1; // snapshot count
    offset = ch->i * l; // start position of the char

    if (ch->isfreq) {

        blk = _blkoffset_lookup(&t->occ_freq[offset], l, occ);
        if (blk > 0) // for block 0, occ starts from 0, not stored
            *occ_start = t->occ_freq[offset + blk - 1]; // char start + snapshot index

    } else {
        unsigned long buf[1024];
        unsigned int bl, rl, i; //buf len, remaining len
        // same logic

        fseek(t->ifp, t->occ_infreq_pos + offset * sizeof (unsigned long), SEEK_SET);

        // _blkoffset_lookup() by buf
        blk = 0;
        rl = l;
        do {

            if (rl > 1024)
                rl -= bl = 1024;
            else
                rl -= bl = rl;

            fread(buf, sizeof (unsigned long), bl, t->ifp);
            blk += i = _blkoffset_lookup(buf, bl, occ);
            if (i < bl) // before the last snapshot within the current buf
                break;

        } while (rl > 0);

        if (blk > 0) {

            fseek(t->ifp, t->occ_infreq_pos + (offset + blk - 1) * sizeof (unsigned long), SEEK_SET);
            fread(occ_start, sizeof (unsigned long), 1, t->ifp);

        }

    }

    if (blk == 0)
        *occ_start = 0;
    *blk_offset = blk * t->block_width;

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
