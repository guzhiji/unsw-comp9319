
#ifndef _STRBUF_H_
#define _STRBUF_H_

#include <stdio.h>

#define STRBUF_LEN 64

typedef struct _strbuf_node strbuf_node;
struct _strbuf_node {
    unsigned char l; // length
    unsigned char c[STRBUF_LEN]; // char array
    strbuf_node * p; // preceeding unit
    strbuf_node * n; // next unit
};

typedef struct {
    strbuf_node * head;
    strbuf_node * tail;
} strbuf;

strbuf * strbuf_init();

void strbuf_free(strbuf * buf);

void strbuf_putchar(strbuf * buf, unsigned char c);

void strbuf_dump(strbuf * buf, FILE * fout);

void strbuf_dump_rev(strbuf * buf, FILE * fout);

#endif

