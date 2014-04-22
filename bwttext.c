
#include "bwttext.h"
#include <stdlib.h>

int cmp_char(const void * c1, const void * c2) {
    // return (int) * (char *) c1 - (int) * (char *) c2;
    return (int) * (unsigned char *) c1 - (int) * (unsigned char *) c2;
}

bwttext * bwttext_read(FILE * fp) {

    int c;
    unsigned long pos = 0, sbefore;
    unsigned char prev_c;
    unsigned char cur_c;
    unsigned char chars[256];
    unsigned char * cur_cp = chars;
    bwttext * t = (bwttext *) malloc(sizeof(bwttext));
    character * chobj;
    bwtindex_char * cur_ch = t->char_table;
    chargroup * prev_cg = NULL;
    while ((c = fgetc(fp)) != EOF) {
        cur_c = (unsigned char) c;

        // char frequency
        chobj = t->char_hash[c];
        if (chobj == NULL) {
            cur_ch->c = *cur_cp = cur_c;
            cur_ch->frequency = 1;
            chobj = (character *) malloc(sizeof(character));
            chobj->info = cur_ch;
            //TODO check
            //chobj->chargroup_list_positions
            //chobj->grouplist
            t->char_hash[c] = chobj;
            t->char_num++;
            cur_ch++;
            cur_cp++;
        } else {
            cur_ch->frequency++;
        }

        // continuous chars
        if (prev_cg == NULL) {
            // chargroup not initialized
            prev_cg = (chargroup *) malloc(sizeof(chargroup));
            prev_cg->start = pos;
            prev_cg->size = 1;
            prev_c = cur_c;
        } else if (prev_c != cur_c) {
            // a new char comes
            // store the prev one
            chargroup_list_add(t, prev_c, prev_cg);
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


