
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
    fwrite(&t->char_num, sizeof(unsigned int), 1, t->ifp);
    fwrite(t->char_table, sizeof(character), t->char_num, t->ifp);
    fgetpos(t->ifp, &p_end);
    rewind(t->ifp);
    fwrite(&p_start, sizeof(unsigned long), 1, t->ifp);
    fsetpos(t->ifp, &p_end);
}

void chartable_load(bwttext * t) {
    fpos_t p_origin;
    unsigned long p_start;

    fgetpos(t->ifp, &p_origin);
    rewind(t->ifp);
    fread(&p_start, sizeof(unsigned long), 1, t->ifp);
    fseek(t->ifp, p_start, SEEK_SET);
    fread(&t->char_num, sizeof(unsigned int), 1, t->ifp);
    fread(t->char_table, sizeof(character), t->char_num, t->ifp);
    fsetpos(t->ifp, &p_origin);
}

