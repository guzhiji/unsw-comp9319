
#ifndef _LZW_H_
#define _LZW_H_

#include <stdlib.h>
#include <stdio.h>

#include "hashtable.h"
#include "symbol.h"

typedef struct _lzw_char lzw_char;
typedef struct _lzw_string lzw_string;

struct _lzw_char {
    int data;
    lzw_char * next;
};

struct _lzw_string {
    lzw_char * head;
    lzw_char * tail;
    int length;
    int hash_code;
};

lzw_string * lzw_string_init();
int lzw_string_hash(lzw_string * s);
int lzw_string_hashnew(lzw_string * s, int c);
int lzw_string_append(lzw_string * s, int c);
void lzw_string_print(lzw_string * s, FILE * fp);
void lzw_string_free(lzw_string * s);

void lzw_compress(FILE * fin, FILE * fout, unsigned short w);
int lzw_csize(FILE * fp, unsigned short w);

#endif
