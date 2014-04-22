
/**
 * exarray: an extensible & persistent array
 */

#ifndef _EXARRAY_H_
#define _EXARRAY_H_

#include <stdio.h>

typedef struct _exarray_node exarray_node;

struct _exarray_node {
    void ** arr;
    unsigned int size;
    unsigned int capacity;
    exarray_node * next;
};

typedef struct {
    exarray_node * head;
    exarray_node * tail;
    unsigned int step_size;
    unsigned long unit_size;
} exarray;

typedef struct {
    exarray_node * node;
    unsigned int position;
    void * data;
} exarray_cursor;

/**
 * initializes an exarray.
 *
 * @param initial_size      the size of the first array when exarray initializes
 * @param step_size         the size of a supplementary array
 * @param unit_size         the size of an element in the exarray
 * @return the new exarray instance
 */
exarray * exarray_init(
        unsigned int initial_size,
        unsigned int step_size,
        unsigned long unit_size);

/**
 * correctly releases resources used the exarray.
 */
void exarray_free(exarray * a);

/**
 * calculates the total number of elements in the exarray.
 */
unsigned long exarray_size(exarray * a);

/**
 * adds an element into the exarray, and its capacity 
 * may increase automatically if necessary.
 */
void exarray_add(exarray * a, void * e);

/**
 * adds all elements of another exarray into an exarray
 */
void exarray_addall(exarray * a, exarray * newarray);

/**
 * gets the next element in the exarray, wrapped in 
 * a cursor object.
 */
exarray_cursor * exarray_next(exarray * a, exarray_cursor * cur);

/**
 * save an exarray into a file.
 */
void exarray_save(exarray * a, FILE * f);

/**
 * load an exarray from a file.
 */
exarray * exarray_load(
        FILE * f,
        unsigned int step_size,
        unsigned long unit_size);

//void exarray_load(FILE * f, exarray * a);

#endif

