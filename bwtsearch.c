
#include "bwtsearch.h"

#include "bwttext.h"
#include <stdio.h>
#include <stdlib.h>

unsigned long char_freq(bwttext * t, character * ch_o) {
    character * ch;
    unsigned int i;

    if (ch_o == NULL) return 0; //non-existent char

    //find next char alphabetically
    for (i = (unsigned int) ch_o->c + 1; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue; //skip non-existent chars
        return ch->ss - ch_o->ss;
    }

    return t->file_size - ch_o->ss; //ch_o is the largest
}

fpos_range * search_fpos_range(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    //dump_chartable(t);
    //dump_pos(t);

    r = (fpos_range *) malloc(sizeof (fpos_range));

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

    if (!bwtblock_find(t, pos, c, &blk)) {// weird if so
        fprintf(stderr, "error: char code=%c; pos=%lu\n - block cannot be found", c, pos);
        return 0;
    }

    if (blk.pl > 0) {// a pure block
/*
        printf("reading a pure block: %d, %lu - req %d, %lu\n", blk.c, blk.pos, c, pos);
*/
/*
        if (blk.c != c) return blk.occ;
*/
        if (blk.c == c) // all c in the block
            return blk.occ + pos - blk.pos; //get by position calculation
        else // no c in the block
            return blk.occ; // occ at the beginning
    }
    // otherwise impure or length not determined yet

    // scan bwttext for c from blk.pos within |blk.pl|
    o = blk.occ;
    fseek(t->fp, 4 + blk.pos, SEEK_SET);
    len = 0;
    p = blk.pos;
    while ((tc = fgetc(t->fp)) != EOF) {
        //if (p++ == pos) return o;
        // TODO
        if (p++ == pos) {
/*
            if (blk.pl > 0) printf("pureblock: %lu, %lu, %lu\n", o, blk.occ, o-blk.occ);
*/
/*
            if (blk.pl>0) printf("pure block: char %d,%d, %d>=%lu=%lu, %lu-1=%lu>=%lu\n", blk.c,tc, blk.pl, pos-blk.pos+1, o-blk.occ+1, p, pos,blk.pos);
*/
            return o;
        }
        if (c == (unsigned char) tc) o++;
        if (blk.pl < 0) {
            len++;
            if (len == -blk.pl) {
                //seems an error
                fprintf(stderr, "error: char code=%c; pos=%lu; block=%lu\n", c, pos, blk.pos);
                break;
            }
        }
    }
    // for blk.pl=0
    return o;

}

unsigned char fpos_char(bwttext * t, unsigned long fpos) {
    int i;
    unsigned char p = 0;
    character * c;
    for (i = 0; i < 256; i++) {
        c = t->char_hash[i];
        if (c == NULL) continue;
        if (c->ss > fpos) break;
        p = c->c;
    }
    return p;
}

unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ) {
    int tc;
    unsigned long n = 0, p = 0;
    fseek(t->fp, 4, SEEK_SET);
    while ((tc = fgetc(t->fp)) != EOF) {
        if (tc == c && n++ == occ)
            return p;
        p++;
    }
    return p;
}

void decode_backward(bwttext * t, FILE * fout) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end;
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        //putchar(c);
        fputc(c, fout);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: char code=%d\n", c);
            break; // error
        }
        p = ch->ss + occ(t, c, p);
    } while (p != t->end);
}

void decode_backward_rev(bwttext * t, FILE * fout) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end;
    unsigned long dp = t->file_size - 1;

    fseek(fout, dp, SEEK_SET);
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        fputc(c, fout);
        fseek(fout, -2, SEEK_CUR);
        dp--;
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL || dp < 0) {
            fprintf(stderr, "\nerror: char code=%d\n, bwt pos=%lu, dest pos=%lu", c, p, dp);
            break; // error
        }
        p = ch->ss + occ(t, c, p);
    } while (p != t->end);
}

/**
 * pos_prev is a fpos; it gets its previous char at it's lpos.
 */
void decode_backward_until(bwttext * t, unsigned long pos_prev, unsigned char until) {
    character * ch;
    unsigned char c;
    unsigned long p = pos_prev;
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        putchar(c);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: char code=%d\n", c);
            break; // error
        }
        p = ch->ss + occ(t, c, p);
    } while (p != t->end && until != c);
}

void decode_forward_until(bwttext * t, unsigned long pos, unsigned char until) {
    character * ch;
    unsigned char c;
    unsigned long occ, p = pos;
    while (p != t->end) {
        c = fpos_char(t, p);
        putchar(c);
        if (c == until) return;
        ch = t->char_hash[(unsigned int) c];
        occ = p - ch->ss; // occ for the next char
        p = lpos(t, c, occ);
    }
    fseek(t->fp, 4 + t->end, SEEK_SET);
    c = fgetc(t->fp);
    putchar(c);
}
