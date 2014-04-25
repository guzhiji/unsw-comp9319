
#include "chargroup.h"

#include "exarray.h"
#include "bwttext.h"
#include "bwtindex.h"
#include <stdlib.h>


chargroup_list * chargroup_list_get(bwttext * t, unsigned char c) {
    unsigned int i;
    character * ch;
    chargroup_list * l;

    ch = t->char_hash[(unsigned int) c];
    if (ch->grouplist == NULL) { // not loaded yet

        // release the least frequent ones

        /* while (t->chargroup_num >= CHARGROUP_NUM_THRESHOLD) { */
        while (t->chargroup_num >= CHARGROUP_NUM_KEEP) {
            l = t->char_freqsorted[--t->chargroup_list_num]->grouplist;
            t->chargroup_num -= chargroup_list_free(l);
        }

        // load the chargroup list for c from index file

        bwtindex_chargrouplist_load(t, ch);
        t->chargroup_num += exarray_size(ch->grouplist->groups);

        // insert the new list by frequency

        i = t->chargroup_list_num++; // point to the one after the last
                                     // and increment count
        while (1) {
            if (i >= 1 && t->char_freqsorted[i - 1]->info->frequency < ch->info->frequency) {
                // push i-1 to i
                t->char_freqsorted[i] = t->char_freqsorted[i-1];
                i--;
            } else {
                // insert at i
                t->char_freqsorted[i] = ch;
                break;
            }
        }

    }

    return ch->grouplist;
}

void chargroup_list_add(bwttext * t, unsigned char c, chargroup * cg) {
    character * ch;
    chargroup_list * l;
    bwtindex_chargroup * icg;

    // get the chargroup list for the char c
    ch = t->char_hash[(unsigned int) c];
    l = ch->grouplist;
    if (l == NULL) {
        l = ch->grouplist = chargroup_list_init();
        //t->chargroup_list_num++;
    }
    
    // add the chargroup
    // TODO check truncated?
    icg = (bwtindex_chargroup *) malloc(sizeof(bwtindex_chargroup));
    icg->offset = cg->start;
    icg->occ_before = ch->info->frequency - cg->size;
    exarray_add(l->groups, icg);
    t->chargroup_num++;

    // write to disk if memory usage is over threshold
    if (t->chargroup_num >= CHARGROUP_NUM_THRESHOLD)
        chargroup_list_savereleaseall(t);

}

void chargroup_list_savereleaseall(bwttext * t) {
    character * ch;
    chargroup_list * l;
    unsigned long * pos;
    int i;

    //dump_occ(t);
    
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        l = ch->grouplist;
        if (l == NULL) continue;

        // save data
        pos = (unsigned long *) malloc(sizeof(unsigned long));
        *pos = ftell(t->ifp);
        //printf("put file pos = %lu\n", *pos);
        exarray_add(ch->chargroup_list_positions, pos);
        bwtindex_chargrouplist_save(l, t->ifp);

        // release resources
        exarray_free(l->groups);
        free(l);
        ch->grouplist = NULL;

    }
    //t->chargroup_list_num = 0;
    t->chargroup_num = 0;
}

chargroup_list * chargroup_list_init() {

    chargroup_list * l = (chargroup_list *) malloc(sizeof(chargroup_list));
    l->groups = exarray_init(
            CHARGROUP_LIST_SIZE_INIT,
            CHARGROUP_LIST_SIZE_STEP,
            sizeof(bwtindex_chargroup));
    return l;

}

unsigned long chargroup_list_free(chargroup_list * l) {
    unsigned long s;
    s = exarray_free(l->groups);
    free(l);
    return s;
}

