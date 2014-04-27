
/**
 * bwttext: processes BWT-encoded text
 */

#ifndef _BWTTEXT_H_
#define _BWTTEXT_H_

#include <stdio.h>
#include "exarray.h"

typedef struct {
    unsigned char c;
    unsigned long frequency;
    //unsigned int chargroup_lists;
} bwtindex_char;

typedef struct {
    //unsigned long position_base; // + offset = real position
    //unsigned int last_chargroup_size; // for checking boundary
    exarray * groups; // bwtindex_chargroup
} chargroup_list;

typedef struct {
    bwtindex_char * info;
    unsigned long smaller_symbols;
    exarray * chargroup_list_positions; // unsigned long
    //exarray * chargroup_lists; // chargroup_list
    chargroup_list * grouplist;
} character;

typedef struct {
    unsigned long start;
    unsigned long size;
} chargroup;

typedef struct {
    unsigned long start;
    unsigned long size;
    unsigned long occ_before;
} bwtindex_chargroup;

typedef struct {
    unsigned int char_num;
    bwtindex_char char_table[256];
    character * char_hash[256];
    character * char_freqsorted[256];
    unsigned long chargroup_num; // up to CHARGROUP_NUM_THRESHOLD
    unsigned int chargroup_list_num; // number of lists loaded
    unsigned long end_position;
//    unsigned long filesize;
    FILE * fp;
    FILE * ifp;
} bwttext;

character * character_init(bwtindex_char * c);

void character_free(character * c);

bwttext * bwttext_init(char * bwtfile, char * indexfile, int buildindex);

void bwttext_read(bwttext * t);

void bwttext_free(bwttext * t);

#endif

