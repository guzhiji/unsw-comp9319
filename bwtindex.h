
/**
 * bwtindex: reads and writes index files
 */

#ifndef _BWTINDEX_H_
#define _BWTINDEX_H_

#include "bwttext.h"
#include <stdio.h>

/********************************
 * Index File Format
 ********************************
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
 * [exarray
 *      [bwtindex_chargroup]
 *      ...
 * ]
 * ...
 ********************************/

void bwtindex_chartable_load(bwttext * t);

void bwtindex_chartable_presave(bwttext * t);

void bwtindex_chartable_save(bwttext * t);

void bwtindex_chargrouplist_load(bwttext * t, character * ch);

void bwtindex_chargrouplist_save(chargroup_list * l, FILE * f);

#endif
