
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned long * arr;
    unsigned long max;
    unsigned long len;
} bucket;

FILE * bwt_in;
unsigned long bwt_len;
unsigned char * bwt_text;

unsigned long bwt_str_len(FILE * in) {
    unsigned long l = 0;
    rewind(in);
    while (fgetc(in) != EOF)
        l++;
    return l;
}

void bwt_str_load(FILE * in, int loadall) {

    bwt_len = bwt_str_len(in);
    if (loadall) {
        bwt_text = (unsigned char *) malloc(sizeof (unsigned char) * bwt_len);
        rewind(in);
        fread(bwt_text, sizeof (unsigned char), bwt_len, in);
    } else {
        bwt_in = in;
        bwt_text = NULL;
    }

}

void bwt_str_unload() {
    if (bwt_text != NULL) free(bwt_text);
}

unsigned char bwt_str_read(unsigned long pos) {

    if (bwt_text != NULL)
        return bwt_text[pos];
    else {
        unsigned char c;
        fseek(bwt_in, pos, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, bwt_in);
        return c;
    }

}

//-------------------------------------------------

bucket * bucket_init() {
    bucket * b = (bucket *) malloc(sizeof (bucket));
    b->len = 0;
    b->max = 32;
    b->arr = (unsigned long *) malloc(sizeof (unsigned long) * b->max);
    return b;
}

void bucket_put(bucket * s, unsigned long n) {

    if (s->len == s->max) {

        unsigned long * nb;
        nb = (unsigned long *) realloc(s->arr, sizeof (unsigned long) * (s->max + 32));

        if (nb == NULL) return;

        s->max += 32;
        s->arr = nb;
    }

    s->arr[s->len++] = n;

}

void bucket_free(bucket * s) {
    free(s->arr);
    free(s);
}

int _cmp_by_str(const void * s1, const void * s2) {
    unsigned char c1, c2;
    unsigned long p1, p2, i;

    p1 = *(unsigned long *) s1;
    p2 = *(unsigned long *) s2;

    for (i = 0; i < bwt_len; i++) {
        c1 = bwt_str_read(p1);
        c2 = bwt_str_read(p2);

        if (c1 > c2) return 1;
        if (c1 < c2) return -1;
        // if equal, compare next pair of chars

        // reaching the end, go back to the beginning
        if (++p1 == bwt_len) p1 = 0;
        if (++p2 == bwt_len) p2 = 0;
    }
    return 0;
}

void bucket_sort(bucket * s) {
    qsort(s->arr, s->len, sizeof (unsigned long), _cmp_by_str);
}

//-------------------------------------------------

/**
 * positional Burrows-Wheeler transform
 * special_char:
 * - 0:  non-positional
 * - >0: e.g. '[', ' ', whose original sequence is kept
 */
unsigned long pbwt(FILE * in, FILE * out, unsigned char special_char, int output_last, int loadall) {

    bucket * bkts[256] = {NULL};
    unsigned long p, ci, last = 0;
    int c, bi, bci;

    bwt_str_load(in, loadall);

    // rotate the string by getting the 
    // start position of each rotation
    // and the start positions are stored 
    // alphabetically separately in buckets
    p = 0;
    while (p < bwt_len) {
        c = bwt_str_read(p);
        if (bkts[c] == NULL)
            bkts[c] = bucket_init();
        bucket_put(bkts[c], p++);
    }

    // sort each bucket so that the whole 
    // string is sorted
    // UPDATE: keep the original sequence of the special char
    for (bi = 0; bi < 256; bi++)
        if (bkts[bi] != NULL)
            if (bi != special_char || special_char == 0)
                bucket_sort(bkts[bi]);

    // reserve a slot for storing position of the last char
    if (output_last)
        fseek(out, sizeof (unsigned long), SEEK_SET);

    ci = 0;
    bi = special_char; // put special char at the very first
    while (bi < 256) {
        // read char in the last column into c
        // as the output char at position ci

        bucket * b = bkts[bi];

        if (b != NULL) {
            // the corresponding char should exist

            for (bci = 0; bci < b->len; bci++) {

                p = b->arr[bci];
                if (p == 0) {
                    // first column is exactly the first char of the input
                    // so last column is the last char of the input
                    c = bwt_str_read(bwt_len - 1);
                    last = ci;
                } else
                    c = bwt_str_read(p - 1);

                ci++;
                fputc(c, out);

            }
        }

        if (bi + 1 == special_char) 
            // just the one before the special char
            bi += 2; // skip the special char
        else if (bi != special_char || special_char == 0)
            bi++;
        else // just processed the special char,
            bi = 0; // start from the beginning

    }

    // output position of the last char
    if (output_last) {
        rewind(out);
        fwrite(&last, sizeof (unsigned long), 1, out);
    }

    bwt_str_unload();

    for (bi = 0; bi < 256; bi++)
        if (bkts[bi] != NULL)
            bucket_free(bkts[bi]);

    return last;

}

int main(int argc, char ** argv) {
    if (argc == 3) {

        FILE * in, * out;

        in = fopen(argv[1], "r");
        if (in == NULL) return 1;
        out = fopen(argv[2], "w");
        if (out == NULL) {
            fclose(in);
            return 1;
        }

        //pbwt(in, out, '[', 1, 1);
        pbwt(in, out, ' ', 1, 1);

        fclose(out);
        fclose(in);
        return 0;
    }
    return 1;
}

