
#include <stdlib.h>     /* qsort */
#include <stdio.h>

typedef struct {
    unsigned char c;
    unsigned int smaller_symbols;
    unsigned int frequency;
} character;

typedef struct {
    //char c;
    unsigned long start_pos;
    unsigned int size;
    //unsigned int occ_before;
} char_group;

typedef struct {
    unsigned int char_num;
    character char_table[256];
    character * hashtable[256]; // ref to char_table elements
    //char chars[256]; // to be sorted
    // FILE * fp;
} bwttext;

void bwttext_inithashtable(bwttext * t) {
    character ** htc; // char ref to hashtable
    int c;

    htc = t->hashtable;
    c = 0;
    while (c++ < 256) {
        *htc = NULL;
        htc++;
    }
}

void bwttext_hashchars(bwttext * t) {
    int c;
    character * cch;

    c = 0;
    cch = t->char_table;
    while (c++ < t->char_num) {
        t->hashtable[(unsigned int) cch->c] = cch;
        cch++;
    }
}

int cmp_char(const void * c1, const void * c2) {
    // return (int) * (char *) c1 - (int) * (char *) c2;
    return (int) * (unsigned char *) c1 - (int) * (unsigned char *) c2;
}

bwttext * read_bwttext(FILE * fp) {
    int c, sbefore;
    unsigned char chars[256];
    unsigned char * curchar;
    character * chobj, * cch;
    bwttext * t;

    // initialize
    t = (bwttext *) malloc(sizeof(bwttext));
    t->char_num = 0;
    //bwttext_inithashtable(t);

    // scan the file and count character frequencies
    chobj = t->char_table;
    curchar = chars;
    while ((c = fgetc(fp)) != EOF) {
        cch = t->hashtable[c];
        if (cch == NULL) {
            // not found, so it's new

            // map it to hashtable
            *curchar = chobj->c = (unsigned char) c;
            chobj->frequency = 1;
            t->hashtable[c] = chobj;

            // go to next char
            t->char_num++;
            chobj++;
            curchar++;

        } else
            cch->frequency++;
    }

    // sort characters lexcographically
    qsort(chars, t->char_num, sizeof(unsigned char), cmp_char);

    // calculate smaller symbols using freq 
    // to generate data for the C[] table
    c = 0; // count for boudndary
    sbefore = 0;
    curchar = chars; // smallest char
    while (c++ < t->char_num) {
        //printf("%d\n", *curchar);
        cch = t->hashtable[(unsigned int) *curchar];
        cch->smaller_symbols = sbefore; // update
        sbefore += cch->frequency; // accumulate freq
        curchar++; // a larger char
    }

    return t;
}

int read_char_table(bwttext * t, FILE * fp) {

    // read number of distinct chars
    t->char_num = 0; // clear first
    fread(&t->char_num, sizeof(unsigned int), 1, fp);

    // nothing
    if (t->char_num == 0)
        return 0;
 
    // read char table
    fread(t->char_table, sizeof(character), t->char_num, fp);

    bwttext_inithashtable(t);
    bwttext_hashchars(t);

    return t->char_num;

}

void write_char_table(bwttext * t, FILE * fp) {

    fwrite(&t->char_num, sizeof(unsigned int), 1, fp);
    fwrite(t->char_table, sizeof(character), t->char_num, fp);

}

int main(int argc, char const *argv[]) {
    FILE * fp, * ifp;
    bwttext * t;

    fp = fopen("../tests/bwtsearch/gcc.bwt", "rb");
    t = read_bwttext(fp);
    fclose(fp);

    ifp = fopen("index1.idx", "wb");
    write_char_table(t, ifp);
    fclose(ifp);

    free(t);

    t = (bwttext *) malloc(sizeof(bwttext));

    ifp = fopen("index1.idx", "rb");
    read_char_table(t, ifp);
    fclose(ifp);

    ifp = fopen("index2.idx", "wb");
    write_char_table(t, ifp);
    fclose(ifp);

    free(t);

    return 0;
}

/*

char count
[
    char
    smaller symbols
    ,
    ...
]

[
    char group size
    [
        start pos
        size
        ,
        ...
    ] 
    ,
    ...
]

*/

