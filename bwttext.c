
#include <stdlib.h>     /* qsort */
#include <stdio.h>

typedef struct {
    unsigned char c;
    unsigned int smaller_symbols;
} character;

typedef struct {
    unsigned long start_pos;
    unsigned int size;
    //unsigned int occ_before;
} char_group;

typedef struct {
    unsigned int char_num;
    character char_table[256];
    character * char_hash[256]; // ref to char_table elements
    FILE * fp;
} bwttext;

void bwttext_inithashtable(bwttext * t) {
    character ** htc; // char ref to hashtable
    int c;

    htc = t->char_hash;
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
        t->char_hash[(unsigned int) cch->c] = cch;
        cch++;
    }
}

int cmp_char(const void * c1, const void * c2) {
    return (int) ((character *) c1)->c - (int) ((character *) c2)->c;
}

void bwttext_readbwt(bwttext * t, FILE * fp) {
    int c, sfreq, sbefore;
    character * chobj, * cch;

    // bwttext_inithashtable(t);

    // scan the file and count character frequencies
    chobj = t->char_table;
    while ((c = fgetc(fp)) != EOF) {
        cch = t->char_hash[c];
        if (cch == NULL) {
            // not found, so it's new

            // temporarily hash it
            chobj->c = (unsigned char) c;
            chobj->smaller_symbols = 1; // freq
            t->char_hash[c] = chobj;

            // go to next char
            t->char_num++;
            chobj++;

        } else
            cch->smaller_symbols++;
    }

    // sort characters lexcographically
    qsort(t->char_table, t->char_num, sizeof(character), cmp_char);

    // calculate smaller symbols using freq 
    // to generate data for the C[] table
    c = 0; // count for boudndary
    sbefore = 0;
    cch = t->char_table; // smallest char
    while (c++ < t->char_num) {
        //printf("%d\n", cch->c);
        sfreq = cch->smaller_symbols; // actually freq
        cch->smaller_symbols = sbefore; // correct it
        sbefore += sfreq; // accumulate freq
        cch++; // a larger char
    }

    // re-hash
    bwttext_inithashtable(t);
    bwttext_hashchars(t);

}

int bwttext_chartable_read(bwttext * t, FILE * fp) {

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

void bwttext_chartable_write(bwttext * t, FILE * fp) {

    fwrite(&t->char_num, sizeof(unsigned int), 1, fp);
    fwrite(t->char_table, sizeof(character), t->char_num, fp);

}

int main(int argc, char const *argv[]) {
    FILE * fp, * ifp;
    bwttext * t;

    fp = fopen("../tests/bwtsearch/gcc.bwt", "rb");
    t = (bwttext *) malloc(sizeof(bwttext));
    bwttext_readbwt(t, fp);
    fclose(fp);

    ifp = fopen("bwttext1.idx", "wb");
    write_char_table(t, ifp);
    fclose(ifp);

    free(t);

    t = (bwttext *) malloc(sizeof(bwttext));

    ifp = fopen("bwttext1.idx", "rb");
    read_char_table(t, ifp);
    fclose(ifp);

    ifp = fopen("bwttext2.idx", "wb");
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

