
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdlib.h>
#include <stdio.h>

#ifndef _TABLE_SIZE_
#define _TABLE_SIZE_ 5021
#endif

/**
 * DATA TYPE: a key-value pair in a hashtable
 */
typedef struct _hashtable_value hashtable_value;

struct _hashtable_value {
    hashtable_value * next;
    void * key;
    void * data;
};

/**
 * DATA TYPE: hashtable
 */
typedef struct {
    hashtable_value ** _table;
    unsigned int size;
} hashtable;

/**
 * initialize a hashtable
 */
hashtable * hashtable_init();

/**
 * release all resources used by a hashtable
 */
void hashtable_free(hashtable * t);

/**
 * hash a key
 */
unsigned int hashtable_hash(void * k);

/**
 * put a key-value pair into the hashtable
 */
void hashtable_put(hashtable * t, void * k, void * v);

/**
 * get value by a key
 */
void * hashtable_get(hashtable * t, void * k);

/**
 * set a function for hashing keys
 */
void hashtable_sethashfunc(unsigned int (*h)(void *));

/**
 * set a function for comparing keys equal or not
 */
void hashtable_setcompfunc(int (*c)(void *, void *));

unsigned int hashtable_hash_int(void * k);

int hashtable_comp_int(void * k1, void * k2);

#endif
