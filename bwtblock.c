
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
    int c;
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

}

/**
 * pre-condition: char_num and char_hash are cleared
 */
void bwtblock_scan(bwttext * t) {

    fpos_t opos;
    int c, start;
    short len;

    // pb_: pure block
    int pb_start;
    short pb_len;

    fseek(t->ifp, sizeof (unsigned long) * 2, SEEK_SET);

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

/**
 * compute occ snapshots for impure blocks.
 * pre-conditions:
 * - char_num and char_hash are cleared
 * - bwtblock_scan() called
 */
void bwtblock_occ_compute(bwttext * t) {
    bwtblock sample;
    character * ch;
    fpos_t blk_pos;
    unsigned long bc, occ_pos, occ;
    int ic, i, l;

    bc = 0;
    occ_pos = ftell(t->ifp);
    fseek(t->ifp, sizeof (unsigned long) * 2, SEEK_SET);
    while (bc++ < _bwtblock_count && fread(&sample, sizeof (bwtblock), 1, t->ifp)) {

        // take an occ snapshot before scanning the block
        if (sample.pos > 0) {
            // also no need for the first block
            fgetpos(t->ifp, &blk_pos);
            sample.snapshot = occ_pos; // update the block
            fseek(t->ifp, occ_pos, SEEK_SET); // jump to occ table
            for (i = 0; i < 256; i++) {
                ch = t->char_hash[i];
                occ = ch == NULL ? 0 : ch->ss;
                fwrite(&occ, sizeof (unsigned long), 1, t->ifp);
            }
            occ_pos = ftell(t->ifp); // the next position
            // overwrite the updated block
            fsetpos(t->ifp, &blk_pos);
            fseek(t->ifp, -sizeof (bwtblock), SEEK_CUR);
            fwrite(&sample, sizeof (bwtblock), 1, t->ifp);
        }

        // scan the block
        fseek(t->fp, sample.pos + 4, SEEK_SET);
        l = sample.pl > 0 ? sample.pl : -sample.pl;
        for (i = 0; i < l; i++) {
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
        }

    }
    fseek(t->ifp, occ_pos, SEEK_SET); // end of occ table

}

void bwtblock_index_build(bwttext * t) {
    unsigned long start;
    unsigned int i, len_blocks;
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

// TODO it's an error if it returns NULL (it shouldn't)

int bwtblock_find(bwttext * t, unsigned long pos, unsigned char c, bwtblock * blk) {
    unsigned int i, bc, r, nr;
    unsigned short len, islastindex;
    bwtblock_index * idx;
    bwtblock blks[64];

    // search index
    idx = bwtblock_index_find(t, pos, &islastindex);
    if (idx == NULL) return 0; // actually it's unlikely

    // find the block
    fseek(t->ifp, idx->add, SEEK_SET);
    bc = 0;
    while (bc < t->blk_index_width) {
        nr = t->blk_index_width - bc;
        if (nr > 64) nr = 64;
        r = fread(blks, sizeof (bwtblock), nr, t->ifp);
        bc += r;
        for (i = 0; i < r; i++) {

            if (blks[i].pos > pos) {
                // e.g. pos = -1
                // it fails to find a match at the very beginning
                // it doesn't matter as pos >= 0
                blk = NULL;
                return 0;
            }
            len = blks[i].pl < 0 ? -blks[i].pl : blks[i].pl;
            if (blks[i].pos <= pos && blks[i].pos + len > pos) {
                // found
                if (blks[i].c != c) {
                    // a different char from the first of the block
                    if (blks[i].pos > 0) {
                        fseek(t->ifp, blks[i].snapshot + c * sizeof (unsigned long), SEEK_SET);
                        fread(&blks[i].occ, sizeof (unsigned long), 1, t->ifp);
                    } else // before the first block, occ is always one
                        blks[i].occ = 0;
                }
                *blk = blks[i];
                return 1;
            } else if (islastindex && i == t->blk_index_width - 1) {
                // at the last block of the last width of index, but not found
                // wait, there can be a missing modulo
                // e.g. _bwtblock_count / BWTBLOCK_INDEX_SIZE
                if (blks[i].c != c) {
                    fseek(t->ifp, blks[i].snapshot + c * sizeof (unsigned long), SEEK_SET);
                    fread(&blks[i].occ, sizeof (unsigned long), 1, t->ifp);
                }
                blks[i].pl = 0; // UNKNOWN LENGTH, NEED TO SCAN UNTIL EOF
                *blk = blks[i];
                return 1;
            }

        }
    }

    // also unlikely
    blk = NULL;
    return 0;
}

