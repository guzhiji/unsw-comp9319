
#include "bwtindex.h"

void bwtindex_chartable_init(bwttext * t) {

    t->char_num = 0;
}

void bwtindex_chartable_free(bwttext * t) {
    character * chobj, * tc;
    unsigned int i;

    // release characters
    chobj = t->char_hash;
    for (i = 0; i < t->char_num, i++) {
        tc = chobj;
        chobj++;
        exarray_free(tc->chargroup_list_positions);
        if (tc->chargroup_lists != NULL)
            exarray_free(tc->chargroup_lists);
        free(tc);
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
    for (i = 0; i < len, i++) {
        chobj = (character *) malloc(sizeof(character));
        // chargroup lists
        chobj->chargroup_lists = NULL;
        chobj->chargroup_list_positions = exarray_load(
                t->ifp,,, sizeof(unsigned long));
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
    for (i = 0; i < t->char_num, i++) {
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
    chargroup_list * list;
    unsigned long * pos;
    unsigned int i;

    chobj = t->char_hash[(unsigned int) c];
    // TODO error handling
    if (chobj == NULL) {
        printf("unknown char %d", c);
        return;
    }
    chobj->chargroup_lists = exarray_init(,, sizeof(chargroup_list));

    n = chobj->chargroup_list_positions->head;
    while (n != NULL) {
        pos = (unsigned long *) n->arr;
        for (i = 0; i < n->size; i++) {
            // TODO exarray_add : it's better to get the element and assign
            list = (chargroup_list *) malloc(sizeof(chargroup_list));

            // find the position of the chargroup list
            fseek(t->ifp, *pos, SEEK_SET);

            // read data
            fread(&list->position_base, sizeof(unsigned long), 1, t->ifp);
            list->chargroups = exarray_load(t->ifp,,, sizeof(bwtindex_chargroup));
            fread(&tlist->last_chargroup_size, sizeof(unsigned int), 1, t->ifp);

            exarray_add(chobj->chargroup_lists, list);
            free(list);

            // next list
            pos++;
        }
        // next exarray node
        n = n->next;
    }

}

void bwtindex_chargrouplist_save(bwttext * t, unsigned char c) {

    exarray_node * n;
    character * chobj;
    chargroup_list * list;
    unsigned int i;

    chobj = t->char_hash[(unsigned int) c];
    // TODO error handling

    n = chobj->chargroup_lists->head;
    while (n != NULL) {
        list = (chargroup_list *) n->arr;
        for (i = 0; i < n->size; i++) {
            fwrite(&list->position_base, sizeof(unsigned long), 1, t->ifp);
            exarray_save(list->chargroups, t->ifp);
            fwrite(&list->last_chargroup_size, sizeof(unsigned int), 1, t->ifp);
            list++;
        }
        n = n->next;
    }

}





