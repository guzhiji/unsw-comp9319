
#include "exarray.h"

exarray * exarray_init(
        unsigned int initial_size,
        unsigned int step_size,
        unsigned int unit_size) {

    exarray * a;
    exarray_node * n;

    n = (exarray_node *) malloc(sizeof(exarray_node));
    n->arr = malloc(unit_size * initial_size);
    n->next = NULL;
    n->size = 0;

    a = (exarray *) malloc(sizeof(exarray));
    a->initial_size = initial_size;
    a->step_size = step_size;
    a->unit_size = unit_size;
    a->node_count = 1;
    a->head = a->tail = n;

    return a;
}

void exarray_free(exarray * a) {
    exarray_node * t, * n = a->head;
    while (n != NULL) {
        t = n;
        n = n->next;
        free(t->arr);
        free(t);
    }
    free(a);
}

unsigned long exarray_size(exarray * a) {
    unsigned long l = 0;
    exarray_node * n = a->head;
    while (n != NULL) {
        l += n->size;
        n = n->next;
    }
    return l;
}

void exarray_add(exarray * a, void * e) {

    exarray_node * n = a->tail;
    if ((a->node_count == 1 && n->size == a->initial_size) ||
            (a->node_count > 1 && n->size == a->step_size)) { 
        // the tail node is used up
        // create a new node

        n = (exarray_node *) malloc(sizeof(exarray_node));
        n->next = NULL;
        n->size = 0;
        n->arr = malloc(a->unit_size * a->step_size);

        a->tail->next = n;
        a->tail = n;
        a->node_count++;

    }
    n->arr[n->size++] = *e;

}

void exarray_save(exarray * a, FILE * f) {
    exarray_node * n = a->head;
    while (n != NULL) {
        fwrite(&n->size, sizeof(unsigned int), 1, f);
        fwrite(n->arr, a->unit_size, n->size, f);
        n = n->next;
    }
}

exarray * exarray_load(
        FILE * f,
        unsigned int initial_size,
        unsigned int step_size,
        unsigned int unit_size) {

    unsigned int l = 0;
    fread(&l, sizeof(unsigned int), 1, f);
}

//TODO positioning
/*
TODO
void* bsearch (const void* key, const void* base,
               size_t num, size_t size,
               int (*compar)(const void*,const void*));
*/

