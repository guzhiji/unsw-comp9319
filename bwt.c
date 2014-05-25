
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

int bwt_cmp(unsigned int p1, unsigned int p2) {
    unsigned char c1, c2;
    unsigned int i;
    for (i = 0; i < bwt_len; i++) {
        c1 = bwt_str_read(p1);
        c2 = bwt_str_read(p2);

        if (c1 > c2) return 1;
        if (c1 < c2) return -1;
        // if equal, compare next pair of chars

        // if reaching the end, go back to the beginning
        if (++p1 == bwt_len) p1 = 0;
        if (++p2 == bwt_len) p2 = 0;
    }
    return 0;
}

int bwt_cmp_ss(const void * s1, const void * s2) {
    return bwt_cmp(* (unsigned short *) s1, * (unsigned short *) s2);
}

int bwt_cmp_ii(const void * s1, const void * s2) {
    return bwt_cmp(* (unsigned int *) s1, * (unsigned int *) s2);
}

unsigned int bwt(FILE * in, FILE * out, int output_last, int loadall) {

    unsigned short * matrix_lower;
    unsigned int * matrix_higher;
    unsigned int i1, i2, l1, l2, p1, p2, threshold, last = 0;
    int cmp;
    unsigned char c;

    threshold = 1 << 16 - 1;

    bwt_str_load(in, loadall);

    if (bwt_len > threshold) {
        l1 = threshold;
        l2 = bwt_len - threshold;
    } else {
        l1 = bwt_len;
        l2 = 0;
    }

    matrix_lower = malloc(sizeof(unsigned short) * l1);
    if (l2 > 0)
        matrix_higher = malloc(sizeof(unsigned int) * l2);
    else
        matrix_higher = NULL;

    // rotate the string
    for (i1 = 0; i1 < l1; i1++)
        matrix_lower[i1] = (unsigned short) i1;
    if (l2 > 0)
        for (i2 = 0; i2 < l2; i2++)
            matrix_higher[i2] = threshold + i2;

    // sort
    qsort(matrix_lower, l1, sizeof(unsigned short), bwt_cmp_ss);
    if (l2 > 0)
        qsort(matrix_higher, l2, sizeof(unsigned int), bwt_cmp_ii);

    // skip the slot for the last position
    if (output_last)
        fseek(out, sizeof(unsigned int), SEEK_SET);

    // output
    // TODO merge low and high
    i1 = 0;
    i2 = 0;
    while (1) {

        // compare c1 and c2 at lower[p1] and higher[p2]
        // take a position from p1 and p2 and treat it as p1
        if (i1 < l1)
            p1 = matrix_lower[i1];
        if (i2 < l2)
            p2 = matrix_higher[i2];

        if (i1 < l1 && i2 < l2) {
            cmp = bwt_cmp(p1, p2);
            if (cmp > 0) // c1 > c2
                p1 = p2; // take a smaller c
        } else if (i2 < l2)
            p1 = p2;
        else if (i1 >= l1)
            break;

        // output the corresponding char in the last column
        if (p1 == 0) {
            // first column is exactly the first char of the input
            // so last column is the last char of the input
            c = bwt_str_read(bwt_len - 1);
            last = i1 + i2;
        } else
            c = bwt_str_read(p1 - 1);

        fputc((int) c, out);

        // advance indices
        if (i1 < l1 && i2 < l2) {
            if (cmp > 0) // c1 > c2
                i2++;
            else // c1 <= c2
                i1++;
        } else if (i2 < l2)
            i2++;
        else
            i1++;

    }

    if (output_last) {
        rewind(out);
        fwrite(&last, sizeof(unsigned int), 1, out);
    }

    // release memory
    free(matrix_lower);
    if (matrix_higher != NULL)
        free(matrix_higher);
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

