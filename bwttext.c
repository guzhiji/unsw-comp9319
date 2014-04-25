
#include "bwttext.h"

#include "exarray.h"
#include "chargroup.h"
#include <stdlib.h>

character * character_init(bwtindex_char * c) {
    character * chobj = (character *) malloc(sizeof(character));

    chobj->info = c;
    chobj->smaller_symbols = 0;
    chobj->grouplist = NULL;// loaded when needed
    chobj->chargroup_list_positions = exarray_init(
            CHARGROUP_LIST_POS_SIZE_INIT,
            CHARGROUP_LIST_POS_SIZE_STEP,
            sizeof(unsigned long));

    return chobj;
}

void character_free(character * c) {

    exarray_free(c->chargroup_list_positions);
    if (c->grouplist != NULL)
        chargroup_list_free(c->grouplist);
    free(c);

}

int cmp_char(const void * c1, const void * c2) {
    return (int) * (unsigned char *) c1 - (int) * (unsigned char *) c2;
}

void bwttext_read(bwttext * t) {

    int c;
    unsigned long pos, sbefore;
    unsigned char prev_c;
    unsigned char cur_c;
    unsigned char chars[256];
    unsigned char * cur_cp = chars;
    character * chobj;
    bwtindex_char * cur_ch = t->char_table;
    chargroup * prev_cg = NULL;

    pos = 0;
    while ((c = fgetc(t->fp)) != EOF) {
        cur_c = (unsigned char) c;
        
        // char frequency
        chobj = t->char_hash[c];
        if (chobj == NULL) {
            cur_ch->c = *cur_cp = cur_c;
            cur_ch->frequency = 1;
            t->char_hash[c] = character_init(cur_ch);
            t->char_num++;
            cur_ch++;
            cur_cp++;
        } else {
            chobj->info->frequency++;
        }

        // continuous chars
        if (prev_cg == NULL || prev_c != cur_c) {
            // a new char comes
            if (prev_cg != NULL) {
                // store the prev one
                //printf("%d: %lu, %d\n", prev_c, prev_cg->start, prev_cg->size);
                chargroup_list_add(t, prev_c, prev_cg);
            }
            // re-initialize chargroup for the new char
            prev_cg = (chargroup *) malloc(sizeof(chargroup));
            prev_cg->start = pos;
            prev_cg->size = 1;
            prev_c = cur_c;
        } else {
            prev_cg->size++;// TODO size may overflow
        }

        pos++;

    }

    // the last one
    if (prev_cg != NULL)
        chargroup_list_add(t, prev_c, prev_cg);

    // finish chargroup lists
    chargroup_list_savereleaseall(t);

    // sort characters lexicographically
    qsort(chars, t->char_num, sizeof(unsigned char), cmp_char);

    // calculate smaller symbols using freq 
    // to generate data for the C[] table
    c = 0; // count for boudndary
    sbefore = 0;
    cur_cp = chars; // smallest char
    while (c++ < t->char_num) {
        //printf("%d\n", *cur_cp);
        chobj = t->char_hash[(unsigned int) *cur_cp];
        chobj->smaller_symbols = sbefore; // update
        sbefore += chobj->info->frequency; // accumulate freq
        cur_cp++; // a larger char
    }

}

bwttext * bwttext_init(char * bwtfile, char * indexfile) {
    int i;
    bwttext * t = (bwttext *) malloc(sizeof(bwttext));

    t->fp = fopen(bwtfile, "rb");
    t->ifp = fopen(indexfile, "w+b");
/*
    t->ifp = fopen(indexfile, "r");
*/
    t->char_num = 0;
    t->chargroup_num = 0;
    t->chargroup_list_num = 0;
    fread(&t->end_position, sizeof(unsigned int), 1, t->fp);

    for (i = 0; i < 256; i++) {
        t->char_hash[i] = NULL;
        t->char_freqsorted[i] = NULL;
    }

    return t;
}

void bwttext_free(bwttext * t) {
    character * chobj;
    unsigned int i;

    fclose(t->fp);
    fclose(t->ifp);

    // release all characters
    for (i = 0; i < 256; i++) {
        chobj = t->char_hash[i];
        if (chobj == NULL) continue;
        character_free(chobj);
    }

    free(t);
}

