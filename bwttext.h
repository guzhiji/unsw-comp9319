
#ifndef _BWTTEXT_H_
#define _BWTTEXT_H_

#include <stdio.h>

#define RL_MAXSIZE 1024 * 1024 * 5
#define RL_BLK_MAXSIZE 1024 * 1024

typedef struct {
    unsigned char c;
    unsigned long ss; //smaller_symbols
} character;

typedef struct {
    unsigned long end; //position of the last char
    unsigned long file_size;

    unsigned long rl_size;
    unsigned short rl_l[RL_MAXSIZE];
    unsigned char rl_c[RL_MAXSIZE];

    unsigned short char_num;
    character char_table[256];
    character * char_hash[256];

    FILE * fp; //file pointer to the bwt file
} bwttext;

bwttext * bwttext_init(char * bwtfile, unsigned char special_char);

void bwttext_free(bwttext * t);

#endif

