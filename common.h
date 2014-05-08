
/**
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

//-----------------------------
//bwt file    718026
//index file  351064
//block count 335
//real        0m11.289s
//user        0m4.300s
//sys         0m6.860s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 2512
//#define BWTBLOCK_INDEX_SIZE 360

//-----------------------------
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

//-----------------------------
//bwt file    25090246
//index file  11236200
//block count 10712
//test1:
//real        8m29.178s
//user        2m43.522s
//sys         5m43.905s
//test2:
//real        6m32.799s
//user        3m45.338s
//sys         2m46.794s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 4512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    5009321
//index file  2489744
//block count 2376
//real        1m18.375s
//user        0m45.887s
//sys         0m32.286s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 4512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    718026
//index file  240824
//block count 230
//real        0m13.924s
//user        0m9.453s
//sys         0m4.392s
#define BWTBLOCK_PURE_MIN 360
#define BWTBLOCK_IMPURE_MAX 4512
#define BWTBLOCK_INDEX_SIZE 12000

//-----------------------------
//bwt file    25090246
//index file  12104120
//block count 11550
//test1
//real        5m54.964s
//user        3m13.616s
//sys         2m40.294s
//test2
//real        6m12.712s
//user        3m23.609s
//sys         2m48.399s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 3512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    5009321
//index file  2684672 !!!
//block count 2562
//real        1m10.739s
//user        0m38.718s
//sys         0m31.874s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 3512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    718026
//index file  276456
//block count 264
//real        0m11.957s
//user        0m7.824s
//sys         0m4.112s
//#define BWTBLOCK_PURE_MIN 360
//#define BWTBLOCK_IMPURE_MAX 3512
//#define BWTBLOCK_INDEX_SIZE 12000

//-----------------------------
//bwt file    718026
//index file  324664
//block count 310
//real        0m10.329s
//user        0m6.056s
//sys         0m4.252s
//#define BWTBLOCK_PURE_MIN 512
//#define BWTBLOCK_IMPURE_MAX 2512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    5009321
//index file  2548432 !!!
//block count 2432
//real        1m4.794s
//user        0m33.510s
//sys         0m31.162s
//#define BWTBLOCK_PURE_MIN 512
//#define BWTBLOCK_IMPURE_MAX 2512
//#define BWTBLOCK_INDEX_SIZE 12000

//bwt file    25090246
//index file  12142896  
//block count 11587
//test1
//real        5m18.751s
//user        2m39.542s
//sys         2m38.238s
//test2
//real        5m47.347s
//user        2m55.347s
//sys         2m51.359s
//#define BWTBLOCK_PURE_MIN 512
//#define BWTBLOCK_IMPURE_MAX 2512
//#define BWTBLOCK_INDEX_SIZE 12000


//-----------------------------
//performance seems better, but index is too large
//#define BWTBLOCK_PURE_MIN 10
//#define BWTBLOCK_IMPURE_MAX 2048
//#define BWTBLOCK_INDEX_SIZE 2024

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
