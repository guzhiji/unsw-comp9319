
#include <stdio.h>
#include <stdlib.h>

FILE * bwt_in;
unsigned int bwt_len;
unsigned char * bwt_text;

unsigned int bwt_str_len(FILE * in) {
    unsigned int l = 0;
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

unsigned char bwt_str_read(unsigned int pos) {

    if (bwt_text != NULL)
        return bwt_text[pos];
    else {
        unsigned char c;
        fseek(bwt_in, pos, SEEK_SET);
        fread(&c, sizeof(unsigned char), 1, bwt_in);
        return c;
    }

}

int bwt_cmp(const void * s1, const void * s2) {
    unsigned char c1, c2;
    unsigned int p1, p2, i;
    p1 = * (unsigned int *) s1;
    p2 = * (unsigned int *) s2;
    for (i = 0; i < bwt_len; i++) {
        c1 = bwt_str_read(p1);
        c2 = bwt_str_read(p2);
        if (c1 > c2) return 1;
        if (c1 < c2) return -1;
        // if equal, compare next pair of chars
        if (++p1 == bwt_len) p1 = 0;
        if (++p2 == bwt_len) p2 = 0;
    }
    return 0;
}

unsigned int bwt(FILE * in, FILE * out, int output_last, int loadall) {

    unsigned int * matrix;
    unsigned int i, last = 0;
    unsigned char c;

    // global vars
    bwt_str_load(in, loadall);
    matrix = malloc(sizeof(unsigned int) * bwt_len);

    // rotate the string
    for (i = 0; i < bwt_len; i++)
        matrix[i] = i;

    // sort
    qsort(matrix, bwt_len, sizeof(unsigned int), bwt_cmp);

    if (output_last)
        fseek(out, sizeof(unsigned int), SEEK_SET);

    for (i = 0; i < bwt_len; i++) {
        // read char in the last column into c
        // as the output char at position i
        if (matrix[i] == 0) {
            // first column is exactly the first char of the input
            // so last column is the last char of the input
            c = bwt_str_read(bwt_len - 1);
            last = i;
        } else
            c = bwt_str_read(matrix[i] - 1);
        fputc((int) c, out);
    }

    if (output_last) {
        rewind(out);
        fwrite(&last, sizeof(unsigned int), 1, out);
    }

    free(matrix);
    bwt_str_unload();

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

