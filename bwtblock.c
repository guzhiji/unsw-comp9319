
#include "bwtblock.h"

#define BWTBLOCK_PURITY_MASK (1 << (8 * sizeof(unsigned short) - 1))
#define USHRT_MAX (BWTBLOCK_PURITY_MASK << 1 - 1)
#define BWTBLOCK_MAX_LEN (USHRT_MAX >> 1)

unsigned long _bwtblock_count = 0;

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
            block_pl = pure ? len : -len;
            bwtblock_save(t, block);// and free as well
            block = NULL;
        }
    }

    // process the new char c
    if (c != NULL) {
        if (block == NULL) {
            block = (bwtblock *) malloc(sizeof(bwtblock));
            // first char of the block
            block->pos = ftell(t->fp) - 1;//= p;
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

void bwtblock_save(bwttext * t, bwtblock * b) {
    int i;
    unsigned long occ;
    character * c;

    fwrite(b, sizeof(bwtblock), 1, t->ifp);
    free(b);
    _bwtblock_count++;

    // TODO occ here is occ for the end of the block
    // but we need beginning
    // occ for all possible chars
    // fixed width of 256
    for (i = 0; i < 256; i++) {
        c = t->char_hash[i];
        occ = c == NULL ? 0 : c->ss - 1;
        fwrite(&occ, sizeof(unsigned long), 1, t->ifp);
    }

}

void bwtblock_buildindex(bwttext * t) {
    unsigned long start;
    unsigned int i;
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

    // scan
    for (i = 0; i < t->blk_index_size && i < BWTBLOCK_INDEX_SIZE; i++) {
        t->blk_index[i].add = ftell(t->ifp);
        fread(&sample, sizeof(bwtblock), 1, t->ifp);
        t->blk_index[i].pos = sample.pos;
        if (t->blk_index_width > 1)
            fseek(t->ifp, (t->blk_index_width - 1) * (sizeof(bwtblock) + 256 * sizeof(unsigned long)), SEEK_CUR);
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
    fseek(t->ifp, sizeof(unsigned long), SEEK_SET);
    fread(&start, sizeof(unsigned long), 1, t->ifp);
    fseek(t->ifp, start, SEEK_SET);
    fread(&t->blk_index_width, sizeof(unsigned int), 1, t->ifp);
    fread(&t->blk_index_size, sizeof(unsigned int), 1, t->ifp);
    fread(t->blk_index, sizeof(bwtblock_index), BWTBLOCK_INDEX_SIZE, t->ifp);
    fsetpos(t->ifp, &p_origin);
}

// TODO it's an error if it returns NULL (it shouldn't)
int bwtblock_find(bwttext * t, unsigned long pos, unsigned char c, bwtblock * blk) {
    unsigned int i;
    unsigned short len;
    bwtblock_index * prev, * cur;

    // search index
    prev = NULL;
    cur = t->blk_index;
    for (i = 0; i < t->blk_index_size && i < BWTBLOCK_INDEX_SIZE; i++) {
        if (cur->pos > pos) break;
        prev = cur;
        cur++;
    }
    if (prev == NULL) return 0;// actually it's unlikely

    // find the block
    fseek(t->ifp, prev->add, SEEK_SET);
    for (i = 0; i < t->blk_index_width; i++) {
        fread(blk, sizeof(bwtblock), 1, t->ifp);
        if (blk->pos > pos) {
            // e.g. pos = -1
            // it fails to find a match at the very beginning
            return 0;
        }
        len = blk->pl < 0 ? -blk->pl : blk->pl;
        if (blk->pos <= pos && blk->pos + len > pos) {
            // find the block
            if (blk->c != c) {
                // a different char from the first of the block
                if (c > 0) // find data for the required char
                    fseek(t->ifp, c * sizeof(unsigned long), SEEK_CUR);
                fread(&blk->occ, sizeof(unsigned long), 1, t->ifp);
                blk->c = c;
            }
            return 1;
        }
    }
    // also unlikely
    return 0;
}


