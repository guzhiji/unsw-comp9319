
#include "bwtblock.h"

#include <stdlib.h>
#include <limits.h>

#define BWTBLOCK_MAXSIZE_RATIO 0.4

unsigned long _bwtblock_count_estimated = 0;
unsigned long _bwtblock_count = 0;

void bwtblock_group_compress(bwtblock * blks, int size) {
    int i, p, min, t, pure;
    // find which 2 to be merged (shortest two)
    p = 0;
    min =  blks[0].pl > 0 ? blks[0].pl : - blks[0].pl;
    min +=  blks[1].pl > 0 ? blks[1].pl : - blks[1].pl;
    if (min > 2) {
        for (i = 1; i < size - 1; i++) {
            t = blks[i].pl > 0 ? blks[i].pl : - blks[i].pl;
            t += blks[i + 1].pl > 0 ? blks[i + 1].pl : - blks[i + 1].pl;
            if (min > t) {
                p = i;
                min = t;
                if (min == 2) break;//can't be shorter
            }
        }
    }
    if (min >= SHRT_MAX) {
        fprintf(stderr, "error: bwtblock.pl is too large\n");
        exit(1);
    }
    // merge them
    pure = blks[p].pl > 0 && blks[p + 1].pl > 0;
    blks[p].pl = min;
    if (!pure || blks[p].c != blks[p + 1].c)
        blks[p].pl = - blks[p].pl;//becomes impure
    // compress
    for (i = p + 1; i < size - 1; i++)
        blks[i] = blks[i + 1];
}

/**
 * attempt to make maximum use of allowed index storage.
 */
void bwtblock_prepare(bwttext * t) {

    int ic, ipc, bc;
    unsigned long pos, len;
    bwtblock blks[1024];

    _bwtblock_count_estimated = (unsigned long) (BWTBLOCK_MAXSIZE_RATIO * t->file_size / sizeof(bwtblock));
    _bwtblock_count = 0;

    // start of BWT text content
    fseek(t->fp, sizeof (unsigned long), SEEK_SET);
    // start of the blocks section in index file
    fseek(t->ifp, sizeof (unsigned long) * 2, SEEK_SET);

    pos = 0;
    bc = -1; // will be advanced to 0 at the first char
    len = 0;
    ipc = EOF;
    while ((ic = fgetc(t->fp)) != EOF) {
        if (ic == ipc && blks[bc].pl < SHRT_MAX) {
            // the same as the preceding char
            blks[bc].pl++;
        } else {
            // new char, new block
            if (bc == 1023) {
                // blks[] is full
                // 1 / block count = len / file size
                // len should be at least file size / estimated blocks
                // and because of loss of precision, add one here
                if (len < t->file_size / _bwtblock_count_estimated + 1) {
                    // progress isn't good
                    // compress 2 slots together so there will be one more
                    bwtblock_group_compress(blks, 1024);
                    bc--;
                } else {
                    // finish this group
                    fwrite(&blks, sizeof(bwtblock), 1024, t->ifp);
                    _bwtblock_count += 1024;
                    // start the next group
                    bc = -1;
                    len = 0;
                    //ipc = EOF;
                }
            }
            // new char
            ipc = ic;
            // new block
            bc++;
            blks[bc].c = (unsigned char) ic;
            blks[bc].pos = pos;
            blks[bc].pl = 1;
        }
        pos++;
        len++;
    }
    if (len > 0) {
        // a group hasn't been finished
        fwrite(&blks, sizeof(bwtblock), bc + 1, t->ifp);
        _bwtblock_count += bc + 1;
    }

}

/**
 * scan a block of BWT text for char frequencies
 */
void bwtblock_scan(bwttext * t, bwtblock * blk) {

    int ic, i, l = blk->pl > 0 ? blk->pl : - blk->pl;
    character * ch;

    fseek(t->fp, 4 + blk->pos, SEEK_SET);
    for (i = 0; i < l ; i++) {
        // count char frequency
        ic = fgetc(t->fp);
        if (ic == EOF) {
            fprintf(stderr, "error: finish the file before expected\n");
            exit(1);
        }
        ch = t->char_hash[ic];
        if (ch == NULL) {
            ch = t->char_hash[ic] = &t->char_table[t->char_num++];
            ch->c = (unsigned char) ic;
            ch->ss = 1;
        } else {
            ch->ss++;
        }
        if (i == 0) {
            // update occ for the first char in the block
            blk->occ = ch->c == blk->c ? ch->ss - 1 : ch->ss;
        }
    }

}

/**
 * create a snapshot of occ for all possible chars 
 * (fixed width of 256)
 */
void bwtblock_occ_snapshot(bwttext * t) {
    character * ch;
    unsigned long occ;
    int i;
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        // occ here is char freq before the next block
        occ = ch == NULL ? 0 : ch->ss;
        fwrite(&occ, sizeof (unsigned long), 1, t->ifp);
    }
}

void bwtblock_occ_compute(bwttext * t) {

    fpos_t group_start, occ_pos;
    bwtblock blks[1024];
    int r, i, isfirst = 1;

    fgetpos(t->ifp, &occ_pos);

    // start of the blocks section in index file
    fseek(t->ifp, sizeof (unsigned long) * 2, SEEK_SET);
    do {
        // read a group of blocks
        fgetpos(t->ifp, &group_start);
        r = fread(&blks, sizeof(bwtblock), 1024, t->ifp);
        if (r > 0) {
            for (i = 0; i < r; i++) {
                // only need to scan impure blocks
                if (blks[i].pl < 0) {
                    // create a snapshot before the block
                    if (!isfirst) {
                        // before the first block
                        // occ is definitely 0
                        fsetpos(t->ifp, &occ_pos);
                        blks[i].snapshot = ftell(t->ifp);
                        bwtblock_occ_snapshot(t);
                        fgetpos(t->ifp, &occ_pos);
                    }
                    // scan the block
                    bwtblock_scan(t, &blks[i]);
                }
                isfirst = 0;
            }
            // over write the block group
            fsetpos(t->ifp, &group_start);
            fwrite(&blks, sizeof(bwtblock), r, t->ifp);
        }
    } while (r > 0);

}

/*
void bwtblock_generate(bwttext * t) {

    bwtblock_prepare(t); // a rough scan and prepare blocks
    bwtblock_occ_compute(t);

}
*/

void bwtblock_index_build(bwttext * t) {
    unsigned long start;
    unsigned int i, len_blocks;
    bwtblock sample;

    // start position

    start = ftell(t->ifp);
    fseek(t->ifp, sizeof (unsigned long), SEEK_SET); // second 4 bytes
    fwrite(&start, sizeof (unsigned long), 1, t->ifp);

    // size and width

    if (_bwtblock_count < BWTBLOCK_INDEX_SIZE) {
        t->blk_index_width = 1;
        t->blk_index_size = (unsigned int) _bwtblock_count;
    } else {
        t->blk_index_width = (unsigned int) (_bwtblock_count / BWTBLOCK_INDEX_SIZE);
        t->blk_index_size = BWTBLOCK_INDEX_SIZE;
    }

    // scan for the first blocks within the current widths

    len_blocks = (t->blk_index_width - 1) * sizeof (bwtblock);

    for (i = 0; i < t->blk_index_size; i++) {
        t->blk_index[i].add = ftell(t->ifp);
        fread(&sample, sizeof (bwtblock), 1, t->ifp);
        t->blk_index[i].pos = sample.pos;
        fseek(t->ifp, len_blocks, SEEK_CUR);
    }

    // save

    fseek(t->ifp, start, SEEK_SET);
    fwrite(&t->blk_index_width, sizeof (unsigned int), 1, t->ifp);
    fwrite(&t->blk_index_size, sizeof (unsigned int), 1, t->ifp);
    fwrite(t->blk_index, sizeof (bwtblock_index), BWTBLOCK_INDEX_SIZE, t->ifp);

}

void bwtblock_index_load(bwttext * t) {
    fpos_t p_origin;
    unsigned long start;

    fgetpos(t->ifp, &p_origin);

    // locate
    fseek(t->ifp, sizeof (unsigned long), SEEK_SET);
    fread(&start, sizeof (unsigned long), 1, t->ifp);
    fseek(t->ifp, start, SEEK_SET);
    // load
    fread(&t->blk_index_width, sizeof (unsigned int), 1, t->ifp);
    fread(&t->blk_index_size, sizeof (unsigned int), 1, t->ifp);
    fread(t->blk_index, sizeof (bwtblock_index), BWTBLOCK_INDEX_SIZE, t->ifp);

    fsetpos(t->ifp, &p_origin);
}

bwtblock_index * bwtblock_index_find(bwttext * t, unsigned long pos, unsigned short * islastindex) {
    unsigned int l, p, min, max;

    *islastindex = 0;
    l = t->blk_index_size > BWTBLOCK_INDEX_SIZE ? BWTBLOCK_INDEX_SIZE : t->blk_index_size;
    min = 0;
    max = l - 1;
    while (max - min > 1) {
        p = (min + max) / 2;
        if (t->blk_index[p].pos < pos) {
            min = p;
        } else if (t->blk_index[p].pos > pos) {
            max = p;
        } else {
            return &t->blk_index[p];
        }
    }
    if (t->blk_index[max].pos <= pos) {
        // e.g. max is still l-1
        *islastindex = 1;
        return &t->blk_index[max];
    }
    if (t->blk_index[min].pos > pos)
        // e.g. min is still 0
        return NULL;
    return &t->blk_index[min];

}

int bwtblock_find(bwttext * t, unsigned long pos, unsigned char c, bwtblock * blk) {
    unsigned int i;
    unsigned short len, islastindex;
    bwtblock_index * idx;

    // search index
    idx = bwtblock_index_find(t, pos, &islastindex);
    if (idx == NULL) return 0; // actually it's unlikely

    // find the block
    fseek(t->ifp, idx->add, SEEK_SET);
    for (i = 0; i < t->blk_index_width; i++) { // scan blocks in the width of index
        fread(blk, sizeof (bwtblock), 1, t->ifp);
        if (blk->pos > pos) {
            // e.g. pos = -1
            // it fails to find a match at the very beginning
            // it doesn't matter as pos >= 0
            return 0;
        }
        len = blk->pl < 0 ? -blk->pl : blk->pl;
        if (blk->pos <= pos && blk->pos + len > pos) {
            // found
            if (blk->c != c) {
                // a different char from the first of the block
                if (blk->pos > 0) {
                    fseek(t->ifp, blk->snapshot + c * sizeof(unsigned long), SEEK_SET);
                    fread(&blk->occ, sizeof (unsigned long), 1, t->ifp);
                } else // before the first block, occ is always one
                    blk->occ = 0;
            }
            return 1;
        } else if (islastindex && i == t->blk_index_width - 1) {
            // at the last block of the last width of index, but not found
            // wait, there can be a missing modulo
            // e.g. _bwtblock_count / BWTBLOCK_INDEX_SIZE
            if (blk->c != c) {
                fseek(t->ifp, blk->snapshot + c * sizeof(unsigned long), SEEK_SET);
                fread(&blk->occ, sizeof (unsigned long), 1, t->ifp);
            }
            blk->pl = 0; // UNKNOWN LENGTH, NEED TO SCAN UNTIL EOF
            return 1;
        }
    }
    // also unlikely
    return 0;
}

