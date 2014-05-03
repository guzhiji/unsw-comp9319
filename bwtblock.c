
#include "bwtblock.h"

#include <stdlib.h>
#include <limits.h>

unsigned long _bwtblock_count = 0;
unsigned long _bwttext_pos = 0;

/**
 * pl > 0, it's pure
 * pl < 0, it's impure
 */
void bwtblock_add(bwttext * t, short pl, unsigned char cstart) {

    short l, tl;
    int c, i;
    unsigned long occ;
    character * ch;
    bwtblock blk;

    // scan all chars in the block for char freq and occ
    l = pl < 0 ? -pl : pl;
    tl = 0;
    while (tl < l && (c = fgetc(t->fp)) != EOF) {
        ch = t->char_hash[c];
        if (ch == NULL) {
            // new char
            ch = t->char_hash[c] = &t->char_table[t->char_num++];
            ch->ss = 1; // freq=1
            ch->c = (unsigned char) c;
        } else {
            ch->ss++; // freq++
        }
        if (tl == 0) {
            // take down the first char in the block
            blk.pos = _bwttext_pos;
            blk.c = (unsigned char) c;
            blk.occ = cstart == c ? ch->ss - 1 : ch->ss; // exclude occ for the current char
        }
        tl++;
        _bwttext_pos++;
    }

    if (tl < l) {
        fprintf(stderr, "error: finish the file before expected\n");
    }

    // store block info into the index file
    blk.pl = pl;
    fwrite(&blk, sizeof (bwtblock), 1, t->ifp);
    _bwtblock_count++;

    // create a snapshot
    // occ for all possible chars (inverse order)
    // fixed width of 256
    for (i = 255; i > -1; i--) {
        ch = t->char_hash[i];
        // occ here is char freq before the next block
        // so include occ for the current position
        occ = ch == NULL ? 0 : ch->ss;
        fwrite(&occ, sizeof (unsigned long), 1, t->ifp);
    }

}

void bwtblock_scan(bwttext * t) {

    fpos_t opos;
    int c, start;
    short len;

    // pb_: pure block
    int pb_start;
    short pb_len;

    c = fgetc(t->fp);
    if (c != EOF) {
        start = pb_start = c;
        len = pb_len = 1;

        while ((c = fgetc(t->fp)) != EOF) {
            if (pb_len < BWTBLOCK_PURE_MIN) {
                // no pure block found
                if (pb_start != c) {
                    // reset pure block start
                    pb_start = c;
                    pb_len = 1;
                } else {
                    // a char repeats
                    pb_len++;
                }
                if (len == BWTBLOCK_IMPURE_MAX || len == SHRT_MAX) {
                    // meet the limit for an impure block

                    // add the impure block
                    fgetpos(t->fp, &opos);
                    fseek(t->fp, -1 - (int) len, SEEK_CUR);
                    bwtblock_add(t, -len, (unsigned char) start); //impure: length<0
                    fsetpos(t->fp, &opos);

                    start = pb_start = c;
                    len = pb_len = 1;
                } else {
                    len++;
                }
                if (pb_len >= BWTBLOCK_PURE_MIN) {
                    // when a pure block gets sufficient length
                    // len becomes the length of the impure block
                    // because len might grow larger than SHRT_MAX
                    // as pb_len approaches SHRT_MAX
                    len -= pb_len;
                }
            } else {
                // a pure block found, but it hasn't finished
                if (pb_start != c || pb_len == SHRT_MAX) {
                    // the different char terminates the pure block

                    fgetpos(t->fp, &opos);
                    fseek(t->fp, -1 - (int) pb_len - (int) len, SEEK_CUR);

                    // add the impure block before pb_start
                    if (len > 0)
                        bwtblock_add(t, -len, (unsigned char) start); //impure: length<0
                    // add the pure block
                    bwtblock_add(t, pb_len, (unsigned char) pb_start); //pure: length>0
                    fsetpos(t->fp, &opos);

                    start = pb_start = c;
                    len = pb_len = 1;
                } else {
                    pb_len++;
                }
            }

        }

        // the last block

        if (len > 0)
            fseek(t->fp, -(int) len, SEEK_CUR);
        if (pb_len >= BWTBLOCK_PURE_MIN)
            fseek(t->fp, -(int) pb_len, SEEK_CUR);

        if (len > 0)
            bwtblock_add(t, -len, (unsigned char) start); //impure: length<0
        if (pb_len >= BWTBLOCK_PURE_MIN)
            bwtblock_add(t, pb_len, (unsigned char) pb_start); //pure: length>0

    }

    t->file_size = _bwttext_pos;

    //TODO debug
    printf("block count=%lu\n", _bwtblock_count);
}

void bwtblock_index_build(bwttext * t) {
    unsigned long start;
    unsigned int i, len_blocks, len_snapshots;
    bwtblock sample;

    // start position

    start = ftell(t->ifp);
    fseek(t->ifp, sizeof (unsigned long), SEEK_SET);
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
    // the snapshot for the block being index hasn't been passed
    len_snapshots = t->blk_index_width * sizeof (unsigned long) * 256;

    for (i = 0; i < t->blk_index_size; i++) {
        t->blk_index[i].add = ftell(t->ifp);
        fread(&sample, sizeof (bwtblock), 1, t->ifp);
        t->blk_index[i].pos = sample.pos;
        fseek(t->ifp, len_blocks + len_snapshots, SEEK_CUR);
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
    unsigned int i;
    bwtblock_index * prev, * cur;

    prev = NULL;
    cur = t->blk_index;
    *islastindex = 1; // assume it's the last
    for (i = 0; i < t->blk_index_size && i < BWTBLOCK_INDEX_SIZE; i++) {
        if (cur->pos > pos) { // just passed the possible position
            *islastindex = 0; // so it's not the last
            break;
        }
        prev = cur;
        cur++;
    }
    // the previous position is what we need
    // if it's a full scan of the index, prev is surely the last index
    return prev;
}

// TODO it's an error if it returns NULL (it shouldn't)

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
                    // push back to find occ before the block, not after
                    fseek(t->ifp, -sizeof (bwtblock) - (c + 1) * sizeof (unsigned long), SEEK_CUR);
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
                fseek(t->ifp, -sizeof (bwtblock) - (c + 1) * sizeof (unsigned long), SEEK_CUR);
                fread(&blk->occ, sizeof (unsigned long), 1, t->ifp);
            }
            blk->pl = 0; // UNKNOWN LENGTH, NEED TO SCAN UNTIL EOF
            return 1;
        }
        fseek(t->ifp, 256 * sizeof (unsigned long), SEEK_CUR); // skip the snapshot of occ
    }
    // also unlikely
    return 0;
}

