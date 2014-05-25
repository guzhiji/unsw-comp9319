
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned long * arr;
    unsigned int max;
    unsigned int len;
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
        bwt_text = (unsigned char *) malloc(sizeof(unsigned char) * bwt_len);
        rewind(in);
        fread(bwt_text, sizeof(unsigned char), bwt_len, in);
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
        fread(&c, sizeof(unsigned char), 1, bwt_in);
        return c;
    }

}

//-------------------------------------------------

bucket * bucket_init() {
    bucket * b = (bucket *) malloc(sizeof(bucket));
    b->len = 0;
    b->max = 32;
    b->arr = (unsigned long *) malloc(sizeof(unsigned long) * b->max);
    return b;
}

void bucket_put(bucket * s, unsigned long n) {

    if (s->len == s->max) {

        unsigned long * nb;
        nb = (unsigned long *) realloc(s->arr, sizeof(unsigned long) * (s->max + 32));

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

    p1 = * (unsigned long *) s1;
    p2 = * (unsigned long *) s2;

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

unsigned long bwt(FILE * in, FILE * out, int output_last, int loadall) {

    bucket * bkts[256] = {NULL};
    unsigned long p, last = 0;
    int c, i, j;

    bwt_str_load(in, loadall);

    // rotate the string by getting the 
    // start position of each rotation
    // and the start positions are stored 
    // separately in buckets
    p = 0;
    while (p < bwt_len) {
        c = bwt_str_read(p);
        if (bkts[c] == NULL)
            bkts[c] = bucket_init();
        bucket_put(bkts[c], p++);
    }

    // sort each bucket so that the whole 
    // string is sorted
    for (i = 0; i < 256; i++)
        if (bkts[i] != NULL)
            bucket_sort(bkts[i]);

    // reserve a slot for storing position of the last char
    if (output_last)
        fseek(out, sizeof(unsigned long), SEEK_SET);

    for (i = 0; i < 256; i++) {
        // read char in the last column into c
        // as the output char at position [i][j]
        bucket * b = bkts[i];
        for (j = 0; j < b->len; j++) {

            p = b->arr[j];
            if (p == 0) {
                // first column is exactly the first char of the input
                // so last column is the last char of the input
                c = bwt_str_read(bwt_len - 1);
                last = i;
            } else
                c = bwt_str_read(p - 1);

            fputc(c, out);

        }
    }

    // output position of the last char
    if (output_last) {
        rewind(out);
        fwrite(&last, sizeof(unsigned int), 1, out);
    }

    bwt_str_unload();

    for (i = 0; i < 256; i++)
        if (bkts[i] != NULL)
            bucket_free(bkts[i]);

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

        bwt(in, out, 1, 1);

        fclose(out);
        fclose(in);
    }
    return 0;
}
