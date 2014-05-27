
#include "bwtsearch.h"

#include "bwttext.h"
#include "occ.h"
#include "strbuf.h"
#include "plset.h"

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

fpos_range * search_forward(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    unsigned long o, fp, lp, tp;
    character * ch;
    unsigned char x;
    int pp, tpp;

    if (l < 1) return NULL;
    r = (fpos_range *) malloc(sizeof (fpos_range));
    pp = 0;
    while (pp < l) {

        x = p[pp++];
        ch = t->char_hash[x];
        if (ch == NULL) {
            free(r);
            return NULL;
        }
        o = char_freq(t, ch) - 1;
        fp = ch->ss;
        lp = ch->ss + o;
        tpp = pp - 2;
        while (tpp >= 0) {
            ch = t->char_hash[p[tpp--]];
            tp = ch->ss + occ(t, ch->c, fp);
            lp = ch->ss + occ(t, ch->c, lp + 1) - 1;
            if (tp == lp + 1 && tp < fp) {
                free(r);
                return NULL;
            }
            fp = tp;
        }
        r->first = fp;
        r->last = lp;

    }

    return r;

}

fpos_range * search_backward(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    if (c == NULL) return NULL;

    r = (fpos_range *) malloc(sizeof (fpos_range));
    r->first = c->ss;
    r->last = r->first + char_freq(t, c) - 1;
    //printf("%c: %lu, %lu\n", x, r->first, r->last);

    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) {
            free(r);
            return NULL;
        }

        //printf("[%lu, %lu, %lu]\n", c->ss, occ(t, x, r->first), occ(t, x, r->last + 1) - 1);
        r->first = c->ss + occ(t, x, r->first);
        r->last = c->ss + occ(t, x, r->last + 1) - 1;
        //printf("%c: %lu, %lu\n\n", x, r->first, r->last);

    }

    if (r->first <= r->last) return r;
    free(r);
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
 * not in-use: backward decoding reverses data;
 * but essentially this is the algorithm
 */
void decode_backward(bwttext * t, FILE * fout) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end;
    do {
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: char code=%d\n", c);
            break; // error
        }
        //putchar(c);
        fputc(c, fout);
        p = ch->ss + occ(t, c, p);
    } while (p != t->end);
}

void decode_backward_rev(bwttext * t, FILE * fout) {
    character * ch;
    unsigned char c;
    unsigned long p, dp;
    unsigned char buf[BUF_SIZE];
    int bufcur;

    p = t->end;

    // start from the last buf
    if (t->file_size > BUF_SIZE)
        dp = t->file_size - BUF_SIZE;
    else
        dp = 0;
    fseek(fout, dp, SEEK_SET);

    bufcur = BUF_SIZE - 1;

    do {
        // read
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        buf[bufcur--] = c;

        // write by buf
        if (bufcur == -1) { // buf is just full
            fwrite(buf, sizeof (unsigned char), BUF_SIZE, fout);
            bufcur = BUF_SIZE - 1;
            // start of next buf
            if (dp >= BUF_SIZE) {
                fseek(fout, -BUF_SIZE * 2, SEEK_CUR);
                dp -= BUF_SIZE;
            } else {
                fseek(fout, 0, SEEK_SET);
                dp = 0;
            }
        }

        // write by char
        //fputc(c, fout);
        //fseek(fout, -2, SEEK_CUR);
        //dp--;

        ch = t->char_hash[(unsigned int) c];

        // nothing but error detection
        if (ch == NULL || dp < 0) { // TODO well, dp is unsigned...
            fprintf(stderr, "\nerror: char code=%d\n, bwt pos=%lu, dest pos=%lu", c, p, dp);
            break; // error
        }

        // next position
        p = ch->ss + occ(t, c, p);

    } while (p != t->end);

    // flush buf
    if (bufcur < BUF_SIZE - 1) // something's in buf if cursor is before the very end
        fwrite(&buf[bufcur + 1], sizeof (unsigned char), BUF_SIZE - 1 - bufcur, fout);

}

/**
 * pos is a fpos
 */
unsigned long decode_backward_until(bwttext * t, unsigned long pos, unsigned char until, int inclusive, strbuf * sb) {
    character * ch;
    unsigned char c;
    unsigned long p = pos;
    do {

        // p is a fpos; it gets its previous char in the the last column
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);

        // the char should exist
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: char code=%d\n", c);
            break; // error
        }

        // output the char
        if (until != c || inclusive)
            strbuf_putchar(sb, c);

        // figure out the fpos of the char
        p = ch->ss + occ(t, c, p);

        // break the loop if it meets the char until
        if (until == c) break;

        // stop when the previous char is the final end
        // (looping back to the end)
    } while (p != t->end);

    // note that it is ensured to output an fpos
    return p;
}

unsigned long decode_forward_until(bwttext * t, unsigned long pos, unsigned char until, int inclusive, strbuf * sb) {
    character * ch;
    unsigned char c;
    unsigned long occ, p = pos;
    do {
        // read next char in the first column
        // or the given char at pos
        c = fpos_char(t, p);

        // output the char
        if (c != until || inclusive)
            strbuf_putchar(sb, c);
        // break the loop if it meets the char until
        if (c == until) return p;

        // map the fpos to lpos
        // the next char is at lpos in the first column
        // so p will still be an fpos - an fpos for the next char
        ch = t->char_hash[(unsigned int) c];
        occ = p - ch->ss;
        p = lpos(t, c, occ);

        // no next char for the last position (t->end)
    } while (p != t->end);

    // TODO p==t->end ?
    return p;

}

void decode_range(bwttext * t, unsigned long start_pos, unsigned long end_pos,
        unsigned char delimiter, int post_content, FILE * fout) {
    strbuf * sb1, * sb2;
    unsigned long i;

    if (t->char_num > 1) {
        if (end_pos > t->char_table[1].ss)
            end_pos = t->char_table[1].ss;
        if (start_pos < 1)
            start_pos = 1;
    } else if (t->char_num == 1
            && t->char_table[0].c != delimiter
            && start_pos < 2 && end_pos > 0) {
        start_pos = 1;
        end_pos = 1;
    } else return;

    for (i = start_pos; i <= end_pos; i++) {
        sb1 = strbuf_init();
        sb2 = strbuf_init();

        // decode_backward_until(t, i - 1, delimiter, !post_content, sb1);
        // decode_forward_until(t, i - 1, delimiter, post_content, sb2);
        // strbuf_dump_rev(sb1, fout);
        // strbuf_dump(sb2, fout);

        decode_backward_until(t, i - 1, delimiter, !post_content, sb1);
        strbuf_dump_rev(sb1, fout);

        sb2->direct_out = stdout;
        decode_forward_until(t, i - 1, delimiter, post_content, sb2);

        strbuf_free(sb1);
        strbuf_free(sb2);
    }
}

void search(bwttext * t, unsigned char * p, unsigned int l,
        unsigned char delimiter, int post_content) {

    fpos_range * r = search_backward(t, p, l);
    //fpos_range * r = search_forward(t, p, l);
    if (r != NULL) {
        unsigned long i, p;
        plset * ps = plset_init();

        printf("found between f-l=%lu-%lu\n", r->first, r->last);

        for (i = r->first; i <= r->last; i++) {

            strbuf * sb1 = strbuf_init();
            // p = decode_backward_until(t, i, delimiter, !post_content, sb1);
            p = decode_forward_until(t, i, delimiter, post_content, sb1);
            if (plset_contains(ps, p)) {
                strbuf_free(sb1);
            } else {
                strbuf * sb2 = strbuf_init();
                // decode_forward_until(t, i, delimiter, post_content, sb2);
                decode_backward_until(t, i, delimiter, !post_content, sb2);
                // plset_put(ps, p, sb1, sb2);
                plset_put(ps, p, sb2, sb1);
            }

        }

        plset_sort(ps);
        plset_print(ps, stdout);

        plset_free(ps);
    } else {
        fprintf(stderr, "no results found\n");
    }
    free(r);

}

