
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdlib.h>
#include <stdio.h>

#define _TABLE_SIZE_ 5021

typedef struct _hashtable_value hashtable_value;

struct _hashtable_value {
    hashtable_value * next;
    int key;
    void * data;
};

typedef struct {
    hashtable_value ** _table;
    int size;
} hashtable;

hashtable * hashtable_init();
void hashtable_free(hashtable * t);
int hashtable_hash(unsigned int k);
void hashtable_put(hashtable * t, int k, void * v);
void * hashtable_get(hashtable * t, int k);

#endif
