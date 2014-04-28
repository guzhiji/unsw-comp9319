
/**
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#define BWTBLOCK_PURE_MIN 50
#define BWTBLOCK_IMPURE_MAX 2048
#define BWTBLOCK_INDEX_SIZE 1024

typedef struct {
    unsigned char c;
    unsigned long ss;//smaller_symbols
} character;

typedef struct {
    unsigned long pos;//position of the start
    unsigned long occ;//occ before the start
    unsigned char c;//char at the start
    short pl;//purity & length, >0 => pure, <0 => impure
} bwtblock;

typedef struct {
    unsigned long pos;//position
    unsigned long add;//address of bwtblock's index
} bwtblock_index;

typedef struct {
    unsigned long file_size;
    unsigned int char_num;
    character char_table[256];
    character * char_hash[256];
    unsigned int blk_index_size;// actual size
    unsigned int blk_index_width;
    bwtblock_index blk_index[BWTBLOCK_INDEX_SIZE];
    //bwtblock_index blk_pure_cache[BWTBLOCK_PURE_INDEX_SIZE];
    unsigned long end;//position of the last char
    FILE * fp;//file pointer to the bwt file
    FILE * ifp;//file pointer to the index file
} bwttext;

#endif

