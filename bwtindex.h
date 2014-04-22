
#include "exarray.h"

/********************************
 * Char table:
 * [char_table_pos]
 * ...
 * [char_table_len]
 * [array
 *      [bwtindex_char]
 *      ...
 * ]
 * [exarray
 *      [chargroup list pos]
 *      ...
 * ]
 * [exarray
 *      [chargroup list pos]
 *      ...
 * ]
 * ...
 ********************************
 * CharGroup list:
 * ...
 * [char pos base]
 * [exarray
 *      [bwtindex_chargroup]
 *      ...
 * ]
 * [last chargroup size]
 * ...
 ********************************/

typedef struct {
    unsigned char c;
    unsigned long frequency;
    //unsigned int chargroup_lists;
} bwtindex_char;

typedef struct {
    bwtindex_char * info;
    unsigned long smaller_symbols;
    exarray * chargroup_list_positions; // unsigned long
    exarray * chargroup_lists; // chargroup_list
} character;

typedef struct {
    unsigned int offset;
    unsigned long occ_before;
} bwtindex_chargroup;

typedef struct {
    unsigned long position_base; // + offset = real position
    unsigned int last_chargroup_size; // for checking boundary
    exarray * chargroups; // bwtindex_chargroup
} chargroup_list;

typedef struct {
    unsigned int char_num;
    bwtindex_char char_table[256];
    character * char_hash[256];
    unsigned long chargroup_num; // up to CHARGROUP_NUM_THRESHOLD
    unsigned int chargroup_list_num; // number of lists loaded
    FILE * fp;
    FILE * ifp;
} bwttext;


