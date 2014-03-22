
#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include <stdio.h>
#include "hashtable.h"

/**
 * DATA TYPE: Huffman node
 */
typedef struct _huffman_node huffman_node;

struct _huffman_node {
    int symbol;
    int freq;
    huffman_node * parent; // for Huffman tree
    huffman_node * next; // for enumeration
    huffman_node * higher; // for priority queue
};

/**
 * DATA TYPE: priority queue for Huffman node
 */
typedef struct {
    huffman_node * head;
} huffman_pqueue;

/**
 * push a Huffman node into a priority queue
 */
void huffman_pq_push(huffman_pqueue * pq, huffman_node * n);

/**
 * pop the least frequent Huffman node from a priority queue
 */
huffman_node * huffman_pq_pop(huffman_pqueue * pq);

/**
 * calculate Huffman compression size in bytes
 */
int huffman_csize(FILE * fp);

#endif
