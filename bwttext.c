
#include "bwttext.h"

bwttext * bwttext_read(FILE * fp) {

    int c;
    unsigned long pos = 0;
    unsigned char prev_c;
    unsigned char cur_c;
    bwttext * t = (bwttext *) malloc(sizeof(bwttext));
    character * chobj;
    bwtindex_char * cur_ch = t->char_table;
    chargroup * prev_cg = NULL;
    while ((c = fgetc(fp)) != EOF) {
        cur_c = (unsigned char) c;

        // char frequency
        chobj = t->char_hash[c];
        if (chobj == NULL) {
            cur_ch->c = cur_c;
            cur_ch->frequency = 1;
            t->char_table[c] = cur_ch;
            t->char_num++;
            cur_ch++;
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
    curchar = chars; // smallest char
    while (c++ < t->char_num) {
        //printf("%d\n", *curchar);
        cch = t->char_hash[(unsigned int) *curchar];
        cch->smaller_symbols = sbefore; // update
        sbefore += cch->frequency; // accumulate freq
        curchar++; // a larger char
    }

}


