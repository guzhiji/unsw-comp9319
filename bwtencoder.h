
#ifndef _BWTENCODER_H_
#define _BWTENCODER_H_

#include <stdio.h>

FILE * bwt_in;
unsigned long bwt_len;
unsigned char * bwt_text;
unsigned char bwt_special_char;

unsigned long bwt_str_len(FILE * in);

void bwt_str_load(FILE * in, int loadall);

void bwt_str_unload();

unsigned char bwt_str_read(unsigned long pos);

unsigned long pbwt(FILE * in, FILE * out, unsigned char special_char, 
        int output_last, int loadall);

//-------------------------------------------------

typedef struct {
    unsigned long * arr;
    unsigned long max;
    unsigned long len;
} bucket;

bucket * bucket_init();

void bucket_put(bucket * s, unsigned long n);

void bucket_free(bucket * s);

void bucket_sort(bucket * s);

#endif

