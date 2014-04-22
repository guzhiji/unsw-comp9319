
#include "bwtindex.h"

void bwtindex_chartable_init(bwttext * t) {

    t->char_num = 0;
}

void bwtindex_free(bwttext * t) {
    character * chobj, * tc;
    unsigned int i;

    // release characters
    for (i = 0; i < 256; i++) {
        chobj = t->char_hash[i];
        if (chobj == NULL) continue;
        exarray_free(chobj->chargroup_list_positions);
        if (chobj->grouplist != NULL)
            chargroup_list_free(chobj->grouplist);
        free(chobj);
    }
    t->char_num = 0;

}

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

void bwtindex_chargrouplist_load(bwttext * t, unsigned char c) {

    exarray_node * n;
    character * chobj;
    unsigned long * pos;
    unsigned int i;

    chobj = t->char_hash[(unsigned int) c];
    // TODO error handling
    if (chobj == NULL) {
        printf("unknown char %d", c);
        return;
    }

    // for each char group list position
    n = chobj->chargroup_list_positions->head;
    while (n != NULL) {
        pos = (unsigned long *) n->arr;
        for (i = 0; i < n->size; i++) {
            exarray * arr;
            unsigned long posbase = 0;
            unsigned int lastsize = 0;

            // find the position of the chargroup list
            fseek(t->ifp, *pos, SEEK_SET);

            // read data
            fread(&posbase, sizeof(unsigned long), 1, t->ifp);
            arr = exarray_load(t->ifp, , sizeof(bwtindex_chargroup));
            fread(&lastsize, sizeof(unsigned int), 1, t->ifp);

            // add to char group list
            // TODO multiple group lists
            if (chobj->grouplist == NULL)
                chobj->grouplist = chargroup_list_init(c, posbase);
            exarray_addall(chobj->grouplist->groups, arr);
            free(arr);
            chobj->grouplist->last_chargroup_size = lastsize;

            // next chargroup list
            pos++;
        }
        // next exarray node
        n = n->next;
    }

}

void bwtindex_chargrouplist_save(chargroup_list * l, FILE * f) {

    fwrite(&l->position_base, sizeof(unsigned long), 1, f);
    exarray_save(f, l->groups);
    fwrite(&l->last_chargroup_size, sizeof(unsigned int), 1, f);

}

