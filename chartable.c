
#include "chartable.h"

#include <stdio.h>

void chartable_inithash(bwttext * t) {
    int i;
    for (i = 0; i < 256; i++)
        t->char_hash[i] = NULL;
}

int _cmp_char(const void * c1, const void * c2) {
    //return (int) * (unsigned char *) c1 - (int) * (unsigned char *) c2;
    return (int) ((character *) c1)->c - ((character *) c2)->c;
}

/**
 * calculate smaller symbols using freq 
 * to generate data for the C[] table
 */
void chartable_ss_compute(bwttext * t) {

    int c;
    unsigned long sbefore, tsbefore;
    character * cur_ch;

    // sort characters lexicographically
    qsort(t->char_table, t->char_num, sizeof(character), _cmp_char);

    chartable_inithash(t);// set all null to re-hash

    c = 0; // count for boudndary
    sbefore = 0;
    cur_ch = t->char_table; // the smallest
    while (c++ < t->char_num) {
        // re-hash
        t->char_hash[(unsigned int) cur_ch->c] = cur_ch;
        // calculate smaller symbols
        tsbefore = sbefore;
        sbefore += cur_ch->ss; // accumulate freq
        cur_ch->ss = tsbefore; // smaller symbols
        cur_ch++; // a larger char
    }

}

void chartable_save(bwttext * t) {
    fpos_t p_end;
    unsigned long p_start;

    p_start = ftell(t->ifp);

    // save data
    fwrite(&t->file_size, sizeof(unsigned long), 1, t->ifp);
    fwrite(&t->char_num, sizeof(unsigned int), 1, t->ifp);
    fwrite(t->char_table, sizeof(character), t->char_num, t->ifp);

    // save position
    fgetpos(t->ifp, &p_end);
    rewind(t->ifp);
    fwrite(&p_start, sizeof(unsigned long), 1, t->ifp);
    fsetpos(t->ifp, &p_end);

}

void chartable_load(bwttext * t) {
    fpos_t p_origin;
    unsigned long p_start;
    character * ch;
    int i;

    fgetpos(t->ifp, &p_origin);

    // locate
    rewind(t->ifp);
    fread(&p_start, sizeof(unsigned long), 1, t->ifp);
    fseek(t->ifp, p_start, SEEK_SET);

    // load
    fread(&t->file_size, sizeof(unsigned long), 1, t->ifp);
    fread(&t->char_num, sizeof(unsigned int), 1, t->ifp);
    fread(t->char_table, sizeof(character), t->char_num, t->ifp);

    fsetpos(t->ifp, &p_origin);

    // hash chars
    chartable_inithash(t);
    ch = t->char_table;
    for (i = 0; i < t->char_num; i++) {
        t->char_hash[(unsigned int) ch->c] = ch;
        ch++;
    }

}

