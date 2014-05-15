
#include "bwtsearch.h"

#include "bwttext.h"
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
    int ic;
    unsigned long o, o_offset, c_pos;

    ch = t->char_hash[c];
    if (ch == NULL)
        return 0;

    if (pos < t->block_width) {
        // before the first snapshot
        o = 0;
    } else {
        o_offset = occtable_offset(t, ch, pos);
        if (ch->isfreq) {
            o = t->occ_freq[o_offset];
            //            fseek(t->ifp, t->occ_freq_pos + o_offset * sizeof (unsigned long), SEEK_SET);
            //            fread(&o, sizeof (unsigned long), 1, t->ifp);
        } else {
            fseek(t->ifp, t->occ_infreq_pos + o_offset * sizeof (unsigned long), SEEK_SET);
            fread(&o, sizeof (unsigned long), 1, t->ifp);
        }
    }

    c_pos = bwtblock_offset(t, pos);
    if (c_pos == pos) return o;

    fseek(t->fp, 4 + c_pos, SEEK_SET);
    {
        unsigned char buf[1024];
        int r;
        do {
            r = fread(buf, sizeof (unsigned char), 1024, t->fp);
            for (ic = 0; ic < r; ic++) {
                if (pos == c_pos++) return o;
                if (buf[ic] == c) o++;
            }
        } while (r > 0);
    }
    //    while ((ic = fgetc(t->fp)) != EOF) {
    //        if (pos == c_pos++) return o;
    //        if (ic == c) o++;
    //    }
    return o;

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
 * find the first position as the given occurance occ of c 
 * in BWT text (pos - position),
 * the last column in the rotation matrix of the 
 * original text (l - last column).
 */
unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ) {
    character * ch;
    unsigned long n, p; // number of occ, char position

    ch = t->char_hash[c];
    if (ch == NULL) {
        exit(1);
    }

    // locate block based on occ value
    // get initial status of position p and occ n
    bwtblock_offset_lookup(t, ch, occ, &p, &n);

    // count occ until the given occ
    // (the position should be found within the block)
    fseek(t->fp, 4 + p, SEEK_SET);
    {
        unsigned char cblk[1024];
        int r, i;

        do {
            r = fread(cblk, sizeof (unsigned char), 1024, t->fp);
            for (i = 0; i < r; i++) {
                // when c occurs, n is compared against occ before it counts;
                if (cblk[i] == c && n++ == occ)
                    return p; // p is returned before it counts the current position
                p++;
            }
        } while (r > 0);

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
    character * ch;
    unsigned char c;
    unsigned long dp;
    unsigned long p = t->end;
    unsigned char buf[1024];
    int bufcur;

    bufcur = 1023;

    // start from the last buf
    if (t->file_size > 1024)
        dp = t->file_size - 1024;
    else
        dp = t->file_size - 1;
    fseek(fout, dp, SEEK_SET);

    do {
        // read
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        buf[bufcur--] = c;

        // write by buf
        if (bufcur == -1) { // buf is just full
            fwrite(buf, sizeof(unsigned char), 1024, fout);
            bufcur = 1023;
            // start of next buf
            if (dp >= 1024) {
                fseek(fout, -2048, SEEK_CUR);
                dp -= 1024;
            } else {
                fseek(fout, 0, SEEK_SET);
                dp = 0;
            }
        }
        //fputc(c, fout);
        //fseek(fout, -2, SEEK_CUR);
        //dp--;

        ch = t->char_hash[(unsigned int) c];

        // nothing but error detection
        if (ch == NULL || dp < 0) {
            fprintf(stderr, "\nerror: char code=%d\n, bwt pos=%lu, dest pos=%lu", c, p, dp);
            break; // error
        }

        // next position
        p = ch->ss + occ(t, c, p);

    } while (p != t->end);

    // flush buf
    if (bufcur < 1023) // something's in buf if cursor is before the very end
        fwrite(&buf[bufcur + 1], sizeof(unsigned char), 1023 - bufcur, fout);

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

