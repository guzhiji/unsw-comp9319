
/**
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

#define OCCTABLE_MEMORY 1024 * 1024 * 5
#define BUF_SIZE 1024 * 1024

typedef struct {
    unsigned char c;
    unsigned long ss; //smaller_symbols
    unsigned char i; //char index in occ table
    unsigned char isfreq; //freq table or infreq table
} character;

typedef struct {
    unsigned long end; //position of the last char

    unsigned long file_size;
    unsigned long block_num;
    unsigned long block_width;

    unsigned long * occ_freq;
    unsigned long occ_infreq_pos;
    unsigned long occ_freq_pos;

    unsigned short char_freq_num;
    unsigned short char_num;
    character char_table[256];
    character * char_hash[256];

    FILE * fp; //file pointer to the bwt file
    FILE * ifp; //file pointer to the index file
} bwttext;

#endif
