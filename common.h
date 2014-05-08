
/**
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

//bwt file    718026
//index file  351064
//block count 335
//real        0m11.289s
//user        0m4.300s
//sys         0m6.860s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 2512
//#define BWTBLOCK_INDEX_SIZE 360

//bwt file    5009321
//index file  2490736
//block count 2376
//real        1m22.126s
//user        0m33.050s
//sys         0m48.691s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 4512
//#define BWTBLOCK_INDEX_SIZE 2500

//bwt file    718026
//index file  258984
//block count 230
//real        0m13.559s
//user        0m6.376s
//sys         0m7.072s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 4512
//#define BWTBLOCK_INDEX_SIZE 2500

//bwt file    25090246
//index file  11236200
//block count 10712
//real        8m29.178s
//user        2m43.522s
//sys         5m43.905s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 4512
//#define BWTBLOCK_INDEX_SIZE 12000

//performance seems better, but index is too large
#define BWTBLOCK_PURE_MIN 10
#define BWTBLOCK_IMPURE_MAX 2048
#define BWTBLOCK_INDEX_SIZE 2024

typedef struct {
    unsigned char c;
    unsigned long ss; //smaller_symbols
} character;

typedef struct {
    unsigned long pos; //position of the start
    unsigned long snapshot; //position of occ snapshot before the block
    unsigned long occ; //occ before the start
    unsigned char c; //char at the start
    short pl; //purity & length, >0 => pure, <0 => impure
} bwtblock;

typedef struct {
    unsigned long pos; //position
    unsigned long add; //address of bwtblock's index
} bwtblock_index;

typedef struct {
    unsigned long file_size;
    unsigned long end; //position of the last char
    unsigned int char_num;
    character char_table[256];
    character * char_hash[256];
    unsigned int blk_index_size; // actual size
    unsigned int blk_index_width;
    bwtblock_index blk_index[BWTBLOCK_INDEX_SIZE];
    //bwtblock_index blk_pure_cache[BWTBLOCK_PURE_INDEX_SIZE];
    FILE * fp; //file pointer to the bwt file
    FILE * ifp; //file pointer to the index file
} bwttext;

#endif
