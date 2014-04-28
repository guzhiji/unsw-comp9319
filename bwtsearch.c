
#include "bwtsearch.h"

#include "bwttext.h"
#include <stdio.h>
#include <stdlib.h>

void dump_chartable(bwttext * t) {
    int i;
    character * ch;
    printf("==================\n");
    printf("dump_chartable:\n");
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        printf("%c (%d): ss=%lu\n", ch->c, ch->c, ch->ss);
    }
    printf("==================\n");
}

unsigned long char_freq(bwttext * t, character * ch_o) {
    character * ch;
    unsigned int i;

    if (ch_o == NULL) return 0;//non-existent char

    //find next char alphabetically
    for (i = (unsigned int) ch_o->c + 1 ; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;//skip non-existent chars
        return ch->ss - ch_o->ss;
    }

    return t->file_size - ch_o->ss;//ch_o is the largest
}

fpos_range * search_range(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    //dump_chartable(t);
    //dump_pos(t);
    
    r = (fpos_range *) malloc(sizeof(fpos_range));

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    if (c == NULL) return NULL;

    r->first = c->ss;
    r->last = r->first + char_freq(t, c) - 1;
    //printf("%c: %lu, %lu\n", x, r->first, r->last);
    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) return NULL;
        //printf("[%lu, %lu, %lu]\n", c->ss, occ(t, x, r->first), occ(t, x, r->last + 1) - 1);
        r->first = c->ss + occ(t, x, r->first);
        r->last = c->ss + occ(t, x, r->last + 1) - 1;
        //printf("%c: %lu, %lu\n\n", x, r->first, r->last);
    }

    if (r->first <= r->last) return r;
    return NULL;

}

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    bwtblock blk;
    unsigned long o, p;
    unsigned short len;
    int tc;

    if (!bwtblock_find(t, pos, c, &blk)) // weird if so
        return 0;

    if (blk.pl > 0) {// a pure block
        if (blk.c == c) // all c in the block
            return blk.occ + pos - blk.pos;//get by position calculation
        else // no c in the block
            return blk.occ; // occ at the beginning
    }
    // otherwise impure or length not determined yet

    // scan bwttext for c from blk.pos within |blk.pl|
    o = blk.occ;
    fseek(t->fp, 5 + blk.pos, SEEK_SET);//one char next to the start of the block, e.g. 5=4+1
    len = 1;//the first char is read and skipped already
    p = blk.pos + 1;
    while ((tc = fgetc(t->fp)) != EOF) {
        if (++p == pos) return o;
        if (c == tc) o++;
        if (blk.pl < 0) {
            len++;
            // blk.pl=-1 is an exception
            // but impossible
            // because an impure block has to be
            // longer than 1
            if (len == -blk.pl) {
                //seems an error
                fprintf(stderr, "error: \n");
                break;
            }
        }
    }
    // for blk.pl=0
    return o;

}

void decode_backword(bwttext * t) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end;
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        putchar(c);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: %d\n", c);
            break; // error
        }
        p = ch->ss + occ(t, c, p);
    } while (p != t->end);
}

