
#include "chartable.h"

#include <stdio.h>

void chartable_inithash(bwttext * t) {
    int i;
    for (i = 0; i < 256; i++)
        t->char_hash[i] = NULL;
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

