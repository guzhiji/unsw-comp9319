
#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include <stdio.h>
#include "hashtable.h"

typedef struct _huffman_node huffman_node;

struct _huffman_node {
    int symbol;
    int freq;
    huffman_node * parent;
    huffman_node * next;
    huffman_node * higher;
};

typedef struct {
    huffman_node * head;
} huffman_pqueue;

void huffman_pq_push(huffman_pqueue * pq, huffman_node * n);

huffman_node * huffman_pq_pop(huffman_pqueue * pq);

int huffman_csize(FILE * fp);

#endif
