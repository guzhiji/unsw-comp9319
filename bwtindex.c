
#include "bwtindex.h"
#include "chargroup.h"
#include "exarray.h"
#include <stdlib.h>

void bwtindex_chartable_load(bwttext * t) {

    unsigned long startpos = 0, smaller;
    unsigned int len = 0, i;
    bwtindex_char * cch;
    character * chobj;

    // find char table
    fread(&startpos, sizeof(unsigned long), 1, t->ifp);
    fseek(t->ifp, startpos, SEEK_SET);
    // read char table
    fread(&len, sizeof(unsigned int), 1, t->ifp);
    t->char_num = len;
    fread(t->char_table, sizeof(bwtindex_char), len, t->ifp);

    smaller = 0;
    cch = t->char_table;
    for (i = 0; i < len; i++) {
        chobj = (character *) malloc(sizeof(character));
        // chargroup list
        chobj->grouplist = NULL;
        chobj->chargroup_list_positions = exarray_load(
                t->ifp,10 , sizeof(unsigned long));
        // smaller symbols for the C[] table
        chobj->smaller_symbols = smaller;
        smaller += cch->frequency;
        // get associated
        chobj->info = cch;
        t->char_hash[(unsigned int) cch->c] = chobj; // for faster access
        // next char
        cch++;
    }

}

void bwtindex_chartable_presave(bwttext * t) {
    unsigned long startpos = 0;
    fwrite(&startpos, sizeof(unsigned long), 1, t->ifp);
}

void bwtindex_chartable_save(bwttext * t) {
    unsigned long startpos;
    unsigned int i;
    bwtindex_char * cch;
    character * chobj;

    // the current position is where it starts
    startpos = ftell(t->ifp);
    // write char table
    fwrite(&t->char_num, sizeof(unsigned int), 1, t->ifp);
    fwrite(t->char_table, sizeof(bwtindex_char), t->char_num, t->ifp);
    // write chargroup list positions
    cch = t->char_table;
    for (i = 0; i < t->char_num; i++) {
        chobj = t->char_hash[(unsigned int) cch->c];
        exarray_save(chobj->chargroup_list_positions, t->ifp);
        cch++;
    }
    // write start position
    fseek(t->ifp, 0, SEEK_SET);
    fwrite(&startpos, sizeof(unsigned long), 1, t->ifp);
}

void bwtindex_chargrouplist_load(bwttext * t, character * chobj) {

    exarray_cursor * cur;
    exarray * arr;
    unsigned long * pos;
    unsigned long posbase;
    unsigned int lastsize;

    cur = NULL;
    while ((cur = exarray_next(chobj->chargroup_list_positions, cur)) != NULL) {

        // find the position of the chargroup list
        pos = (unsigned long *) cur->data;
        fseek(t->ifp, *pos, SEEK_SET);

        // read data
        posbase = 0;
        fread(&posbase, sizeof(unsigned long), 1, t->ifp);
        arr = exarray_load(t->ifp, 10, sizeof(bwtindex_chargroup));
        lastsize = 0;
        fread(&lastsize, sizeof(unsigned int), 1, t->ifp);

        // add to char group list
        if (chobj->grouplist == NULL)
            chobj->grouplist = chargroup_list_init(posbase);
        exarray_addall(chobj->grouplist->groups, arr);
        free(arr);
        chobj->grouplist->last_chargroup_size = lastsize;

    }

}

void bwtindex_chargrouplist_save(chargroup_list * l, FILE * f) {

    fwrite(&l->position_base, sizeof(unsigned long), 1, f);
    exarray_save(l->groups, f);
    fwrite(&l->last_chargroup_size, sizeof(unsigned int), 1, f);

}

