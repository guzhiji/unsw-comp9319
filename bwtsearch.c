
#include "hashtable.h"
#include <stdlib.h>     /* qsort */

typedef struct {
    hashtable * char_table; // map char to charater
    unsigned int char_num;
    char chars[256];
    FILE * fp; // encoding file
    // FILE * ifp; // index file
} bwttext;

typedef struct {
    char c;
    unsigned int frequency;
    unsigned int smaller_symbols;
} character;

typedef struct {
    unsigned int first;
    unsigned int last;
} fpos_range;

int cmp_char(const void * c1, const void * c2) {
    return (int) * (char *) c1 - (int) * (char *) c2;
}

int hashtable_comp_character(void * k1, void * k2) {
    // return (character *) k1->c == (character *) k2->c;
    return * (char *) k1 == * (char *) k2;
}

unsigned int hashtable_hash_character(void * k) {
    // return (unsigned int)(character *) k->c;
    return (unsigned int) * (char *) k;
}

void hashtable_free_character(void * key, void * data) {
    free(key);
    free(data);
}

bwttext * read_bwttext(FILE * fp) {
    int c, cbefore;
    char cc;
    char * chkey, * cch;
    character * chobj;
    bwttext * t;
    
    t = malloc(sizeof(bwttext));
    t->char_num = 0;
    t->char_table = hashtable_init(512);
    cch = t->chars;

    hashtable_setcompfunc(t->char_table, hashtable_comp_character);
    hashtable_sethashfunc(t->char_table, hashtable_hash_character);
    hashtable_setfreefunc(t->char_table, hashtable_free_character);

    // scan the file and count character frequencies
    while ((c = fgetc(fp)) != EOF) {
        cc = (char) c;
        chobj = hashtable_get(t->char_table, &cc);
        if (chobj == NULL) {
            // not found, so it's new

            // create chkey and chobj for storage
            chobj = malloc(sizeof(character));
            chobj->frequency = 1;
            chkey = malloc(sizeof(char));
            // t->chars[t->char_num++] = chobj->c = *chkey = cc;
            *cch = *chkey = chobj->c = cc;
            hashtable_put(t->char_table, chkey, chobj);

            // go to next char
            cch++;
            t->char_num++;

        } else {
            chobj->frequency++;
        }
    }

    // sort characters lexcographically
    qsort(t->chars, t->char_num, sizeof(char), cmp_char);

    // calculate smaller symbols
    // to generate data for the C[] table
    c = 0; // count for boundary
    cbefore = 0; // accumulate freq
    cch = t->chars; // smallest char
    while (c < t->char_num) {
        chobj = hashtable_get(t->char_table, cch);
        chobj->smaller_symbols = cbefore;
        cbefore += chobj->frequency;
        cch++; // a larger char
        c++;
    }

    return t;
}

void free_bwttext(bwttext * t) {
    hashtable_free(t->char_table);
    fclose(t->fp);
    // fclose(t->ifp);
    // free(t->chars);
    free(t);
}

unsigned int occ(bwttext * t, char c, unsigned int pos) {

}

fpos_range * search_range(bwttext * t, char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    char x;

    r = malloc(sizeof(fpos_range));

    pp = l - 1;
    x = p[pp];

    c = hashtable_get(t->char_table, &x);
    r->first = c->smaller_symbols;
    r->last = r->first + c->frequency - 1;

    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = hashtable_get(t->char_table, &x);
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
    }

    if (r->first <= r->last) return r;
    return NULL;

}

int main(int argc, char ** argv) {
    return 0;
}