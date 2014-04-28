
#include "bwtblock.h"

#include <stdlib.h>

#define BWTBLOCK_PURITY_MASK (1 << (8 * sizeof(unsigned short) - 1))
#define USHRT_MAX (BWTBLOCK_PURITY_MASK << 1 - 1)
#define BWTBLOCK_MAX_LEN (USHRT_MAX >> 1)

unsigned long _bwtblock_count = 0;

/**
 * save a the block and also create a snapshot at the end of the block.
 */
void bwtblock_save(bwttext * t, bwtblock * b) {
    int i;
    unsigned long occ;
    character * c;

    // save the block
    fwrite(b, sizeof(bwtblock), 1, t->ifp);
    free(b);
    _bwtblock_count++;

    // create a snapshot
    // occ for all possible chars (inverse order)
    // fixed width of 256
    for (i = 255; i > -1; i--) {
        c = t->char_hash[i];
        occ = c == NULL ? 0 : c->ss; // freq before the next block
        fwrite(&occ, sizeof(unsigned long), 1, t->ifp);
    }

}

/**
 * accept a char each time and automatically create a block.
 */
void bwtblock_addchar(bwttext * t, character * c, unsigned long p) {

    static bwtblock * block = NULL;
    static character * prev_char = NULL;
    static unsigned short len = 0;
    static int pure = 1;// assume TRUE

    // save the block
    if (block != NULL) {
        if (c == NULL ||
                len == BWTBLOCK_MAX_LEN ||
                (pure && prev_char != c && len > BWTBLOCK_PURE_MIN) ||
                (!pure && len == BWTBLOCK_IMPURE_MAX)) {
            // flush (c is null) or
            // reach len capacity or
            // was pure but a different char comes after the min limit or
            // is impure and reach the max limit
            // (excluding the current char)

            //block->pl = (pure ? BWTBLOCK_PURITY_MASK : 0) & len;
            block->pl = pure ? len : -len;
            bwtblock_save(t, block);// and free as well
            block = NULL;
        }
    }

    // process the new char c
    if (c != NULL) {
        if (block == NULL) {
            block = (bwtblock *) malloc(sizeof(bwtblock));
            // first char of the block
            block->pos = p; //ftell(t->fp) - 1;
            block->occ = c->ss - 1;// freq - 1
            block->c = c->c;
            // init/re-init
            prev_char = NULL;
            len = 1;
            pure = 1;
        } else {
            pure = prev_char == c && pure;
            prev_char = c;
            len++;
        }
    }

}

void bwtblock_buildindex(bwttext * t) {
    unsigned long start;
    unsigned int i, len_blocks, len_snapshots;
    bwtblock sample;

    // start position

    start = ftell(t->ifp);
    fseek(t->ifp, sizeof(unsigned long), SEEK_SET);
    fwrite(&start, sizeof(unsigned long), 1, t->ifp);

    // size and width

    if (_bwtblock_count < BWTBLOCK_INDEX_SIZE) {
        t->blk_index_width = 1;
        t->blk_index_size = (unsigned int) _bwtblock_count;
    } else {
        t->blk_index_width = (unsigned int) (_bwtblock_count / BWTBLOCK_INDEX_SIZE);
        t->blk_index_size = BWTBLOCK_INDEX_SIZE;
    }

    // scan for the first blocks within the current widths

    len_blocks = (t->blk_index_width - 1) * sizeof(bwtblock);
    // the snapshot for the block being index hasn't been passed
    len_snapshots = t->blk_index_width * sizeof(unsigned long) * 256;

    for (i = 0; i < t->blk_index_size; i++) {
        t->blk_index[i].add = ftell(t->ifp);
        fread(&sample, sizeof(bwtblock), 1, t->ifp);
        t->blk_index[i].pos = sample.pos;
        fseek(t->ifp, len_blocks + len_snapshots, SEEK_CUR);
    }

    // save

    fseek(t->ifp, start, SEEK_SET);
    fwrite(&t->blk_index_width, sizeof(unsigned int), 1, t->ifp);
    fwrite(&t->blk_index_size, sizeof(unsigned int), 1, t->ifp);
    fwrite(t->blk_index, sizeof(bwtblock_index), BWTBLOCK_INDEX_SIZE, t->ifp);

}

void bwtblock_loadindex(bwttext * t) {
    fpos_t p_origin;
    unsigned long start;

    fgetpos(t->ifp, &p_origin);

    // locate
    fseek(t->ifp, sizeof(unsigned long), SEEK_SET);
    fread(&start, sizeof(unsigned long), 1, t->ifp);
    fseek(t->ifp, start, SEEK_SET);
    // load
    fread(&t->blk_index_width, sizeof(unsigned int), 1, t->ifp);
    fread(&t->blk_index_size, sizeof(unsigned int), 1, t->ifp);
    fread(t->blk_index, sizeof(bwtblock_index), BWTBLOCK_INDEX_SIZE, t->ifp);

    fsetpos(t->ifp, &p_origin);
}

// TODO it's an error if it returns NULL (it shouldn't)
int bwtblock_find(bwttext * t, unsigned long pos, unsigned char c, bwtblock * blk) {
    unsigned int i;
    unsigned short len, lastindex;
    bwtblock_index * prev, * cur;

    // search index
    prev = NULL;
    cur = t->blk_index;
    lastindex = 1; // assume it's the last
    for (i = 0; i < t->blk_index_size && i < BWTBLOCK_INDEX_SIZE; i++) {
        if (cur->pos > pos) { // just passed the possible position
            lastindex = 0; // so it's not the last
            break;
        }
        prev = cur;
        cur++;
    }
    // the previous position is what we need
    if (prev == NULL) return 0;// actually it's unlikely
    // if it's a full scan of the index, prev is surely the last index

    // find the block
    fseek(t->ifp, prev->add, SEEK_SET);
    for (i = 0; i < t->blk_index_width; i++) { // scan blocks in the width of index
        fread(blk, sizeof(bwtblock), 1, t->ifp);
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
                    fseek(t->ifp, - sizeof(bwtblock) - (c + 1) * sizeof(unsigned long), SEEK_CUR);
                    fread(&blk->occ, sizeof(unsigned long), 1, t->ifp);
                } else // before the first block, occ is always one
                    blk->occ = 0;
                blk->c = c;
            }
            return 1;
        } else if (lastindex && i == t->blk_index_width - 1) {
            // at the last block of the last width of index, but not found
            // wait, there can be a missing modulo
            // e.g. _bwtblock_count / BWTBLOCK_INDEX_SIZE
            if (blk->c != c) {
                fseek(t->ifp, - sizeof(bwtblock) - (c + 1) * sizeof(unsigned long), SEEK_CUR);
                fread(&blk->occ, sizeof(unsigned long), 1, t->ifp);
                blk->c = c;
            }
            blk->pl = 0; // UNKNOWN LENGTH, NEED TO SCAN UNTIL EOF
            return 1;
        }
        fseek(t->ifp, 256 * sizeof(unsigned long), SEEK_CUR); // skip the snapshot of occ
    }
    // also unlikely
    return 0;
}


