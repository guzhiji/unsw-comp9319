
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdlib.h>
#include <stdio.h>

/**
 * DATA TYPE: a key-value pair in a hashtable
 */
typedef struct _hashtable_value hashtable_value;

struct _hashtable_value {
    hashtable_value * next;
    hashtable_value * last;
    void * key;
    void * data;
};

/**
 * DATA TYPE: hashtable
 */
typedef struct {
    hashtable_value * _table;
    unsigned int (*_hash)(void *);
    int (*_comp)(void *, void *);
    void (*_free)(void *, void *);
    unsigned int size;
    unsigned int table_size;
} hashtable;

/**
 * initialize a hashtable
 */
hashtable * hashtable_init(const unsigned int tblsize);

/**
 * release all resources used by a hashtable
 */
void hashtable_free(hashtable * t);

/**
 * put a key-value pair into the hashtable
 */
void hashtable_put(hashtable * t, void * k, void * v);

/**
 * get value by a key
 */
void * hashtable_get(hashtable * t, void * k);

/**
 * set a custom function for hashing keys
 */
void hashtable_sethashfunc(hashtable * t, unsigned int (*h)(void *));

/**
 * set a custom function for comparing keys equal or not
 */
void hashtable_setcompfunc(hashtable * t, int (*c)(void *, void *));

/**
 * set a custom function for releasing resources of key and data
 */
void hashtable_setfreefunc(hashtable * t, void (*f)(void *, void *));

#endif

