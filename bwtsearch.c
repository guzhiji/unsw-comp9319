
#include "bwtsearch.h"

#include "bwttext.h"
#include "bwtblock.h"
#include "occtable.h"
#include "strbuf.h"
#include "pset.h"

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

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {

    character * ch;
    bwtblock_start start;
    unsigned long offset;

    ch = t->char_hash[c];

    // a char never occurred
    if (ch == NULL)
        return 0;

    // occ at the beginning of a block
    if (pos < t->block_width) {
        // before the first snapshot
        start.occ = 0;
    } else {
        offset = occtable_offset(t, ch, pos);
        if (ch->isfreq) {
            start.occ = t->occ_freq[offset];
            //fseek(t->ifp, t->occ_freq_pos + offset * sizeof (unsigned long), SEEK_SET);
            //fread(&start.occ, sizeof (unsigned long), 1, t->ifp);
        } else {
            fseek(t->ifp, t->occ_infreq_pos + offset * sizeof (unsigned long), SEEK_SET);
            fread(&start.occ, sizeof (unsigned long), 1, t->ifp);
        }
    }

    // where is the block
    start.offset = bwtblock_offset(t, pos);

    // exactly the beginning, return directly
    if (start.offset == pos) return start.occ;
    // otherwise look into the block
    return bwtblock_occ(t, &start, c, pos);

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

/**
 * find the first position as the given occurence occ of c 
 * in BWT text (pos - position),
 * the last column in the rotation matrix of the 
 * original text (l - last column).
 */
unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ) {
    character * ch;
    bwtblock_start start;

    ch = t->char_hash[c];
    if (ch == NULL) {
        fprintf(stderr, "\nerror: char code=%d\n", c);
        exit(1);
    }

    // locate block based on occ value
    // get initial status of position p and occ n
    bwtblock_start_lookup(t, ch, occ, &start);

    return bwtblock_occ_position(t, &start, c, occ);

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
unsigned long decode_backward_until(bwttext * t, unsigned long pos_prev, unsigned char until, strbuf * sb) {
    character * ch;
    unsigned char c;
    unsigned long p = pos_prev;
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        if (until != c) {
            strbuf_putchar(sb, c);
            ch = t->char_hash[(unsigned int) c];
            if (ch == NULL) {
                fprintf(stderr, "\nerror: char code=%d\n", c);
                break; // error
            }
            p = ch->ss + occ(t, c, p);
        }
    } while (p != t->end && until != c);
    return p;
}

unsigned long decode_forward_until(bwttext * t, unsigned long pos, unsigned char until) {
    character * ch;
    unsigned char c;
    unsigned long occ, p = pos;
    while (p != t->end) {
        c = fpos_char(t, p);
        putchar(c);
        if (c == until) return p;
        ch = t->char_hash[(unsigned int) c];
        occ = p - ch->ss; // occ for the next char
        p = lpos(t, c, occ);
    }
    fseek(t->fp, 4 + t->end, SEEK_SET);
    c = fgetc(t->fp);
    putchar(c);
    return t->end;
}

void search(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r = search_fpos_range(t, p, l);
    if (r != NULL) {
        unsigned long i, p;
        pset * ps = pset_init();

        //printf("found between f-l=%lu-%lu\n", r->first, r->last);

        for (i = r->first; i <= r->last; i++) {

            strbuf * sb = strbuf_init();
            p = decode_backward_until(t, i, '\n', sb);
            if (!pset_contains(ps, p)) {
                pset_put(ps, p);
                strbuf_dump(sb, stdout);
                decode_forward_until(t, i, '\n');
            }
            strbuf_free(sb);

        }

        pset_free(ps);
    } else {
        fprintf(stderr, "no results found\n");
    }
    free(r);

}

