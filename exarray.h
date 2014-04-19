
#ifndef _EXARRAY_H_
#define _EXARRAY_H_

typedef _exarray_node exarray_node;
struct {
    void * arr;
    unsigned int size;
    exarray_node * next;
} _exarray_node;

typedef struct {
    exarray_node * head;
    exarray_node * tail;
    unsigned int node_count;
    unsigned int initial_size;
    unsigned int step_size;
    unsigned int unit_size;
} exarray;

exarray * exarray_init(
        unsigned int initial_size,
        unsigned int step_size,
        unsigned int unit_size);

void exarray_free(exarray * a);

unsigned long exarray_size(exarray * a);

void exarray_add(exarray * a, void * e);

void exarray_save(exarray * a, FILE * f);

exarray * exarray_load(
        FILE * f,
        unsigned int initial_size,
        unsigned int step_size,
        unsigned int unit_size);


#endif

