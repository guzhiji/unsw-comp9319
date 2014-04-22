
#include "exarray.h"
//#include <limits.h>
#include <stdlib.h>
#include <string.h>

exarray * exarray_init(
        unsigned int initial_size,
        unsigned int step_size,
        unsigned long unit_size) {

    exarray * a;
    exarray_node * n = (exarray_node *) malloc(sizeof(exarray_node));
    n->arr = malloc(sizeof(void *) * initial_size);
    n->next = NULL;
    n->size = 0;
    n->capacity = initial_size;

    a = (exarray *) malloc(sizeof(exarray));
    a->step_size = step_size;
    a->unit_size = unit_size;
    a->head = a->tail = n;

    return a;
}

void exarray_free(exarray * a) {
    unsigned int i;
    void * e;
    exarray_node * t, * n = a->head;
    while (n != NULL) {
        t = n;
        n = n->next;
        // TODO specialized free function

        e = t->arr;
        for (i = 0; i < t->size; i++) {
            free(e);
            e++;
        }

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

void exarray_add(exarray * a, const void * e) {

    exarray_node * n = a->tail;
    if (n->size == n->capacity) {
        // the tail node is used up
        // create a new node

        n = (exarray_node *) malloc(sizeof(exarray_node));
        n->next = NULL;
        n->size = 0;
        n->capacity = a->step_size;
        n->arr = malloc(sizeof(void *) * a->step_size);

        a->tail->next = n;
        a->tail = n;

    }
    n->arr[n->size++] = e;

}

// TODO check if newarray is released via exarray_free
void exarray_addall(exarray * a, exarray * newarray) {
    // more specifically, element types should be consistent
    if (a->unit_size == newarray->unit_size) {
        a->tail->next = newarray->head;
        a->tail = newarray->tail;
    }
}

exarray_cursor * exarray_next(exarray * a, exarray_cursor * cur) {
    if (cur == NULL) {
        // the first element, because no cursor means the first access
        // but if it's empty, NULL represents nothing available as the next
        if (a->head->size == 0) return NULL;
        // create the cursor
        cur = (exarray_cursor *) malloc(sizeof(exarray_cursor));
        cur->node = a->head;
        cur->position = 0;
        cur->data = a->head->arr;
        return cur;
    } else if (cur->position + 1 < cur->node->size) {
        // the next element is still in the same node
        cur->position++;
        cur->data++;
        return cur;
    } else if (/* cur->node != a->tail && */ cur->node->next != NULL) {
        // there are nodes afterwards
        cur->node = cur->node->next;
        cur->position = 0;
        cur->data = cur->node->arr;
        return cur;
    } else {
        // otherwise, it's the end
        free(cur);
        return NULL;
    }
}

void exarray_save(exarray * a, FILE * f) {
    unsigned int nc = 0, i;
    exarray_node * n;
    void * e;
    
    n = a->head;
    while (n != NULL) {
        nc++;
        n = n->next;
    }
    fwrite(&nc, sizeof(unsigned int), 1, f);

    n = a->head;
    while (n != NULL) {
        fwrite(&n->size, sizeof(unsigned int), 1, f);
        e = n->arr;
        for (i = 0; i < n->size; i++) {
            fwrite(e, a->unit_size, 1, f);
            e++;
        }
        n = n->next;
    }
}

exarray * exarray_load(
        FILE * f,
        unsigned int step_size,
        unsigned long unit_size) {

    unsigned int nc = 0, l, i, j;
 
    exarray * a;
    exarray_node * n, * p = NULL;

    a = (exarray *) malloc(sizeof(exarray));
    a->step_size = step_size;
    a->unit_size = unit_size;

    fread(&nc, sizeof(unsigned int), 1, f);
    for (i = 0; i < nc; i++) {
        l = 0;
        fread(&l, sizeof(unsigned int), 1, f);

        n = (exarray_node *) malloc(sizeof(exarray_node));
        n->size = n->capacity = l;
        n->arr = malloc(sizeof(void *) * l);
        for (j = 0; j < l; j++) {
            n->arr[j] = malloc(unit_size);
            fread(&n->arr[j], unit_size, 1, f);
        }

        n->next = NULL;
        if (p == NULL) // no previous, so it's the first
            a->head = n;
        else // the previous's next is the current node
            p->next = n;
        p = n;
    }
    a->tail = n;

    return a;
}

/*
TODO
void* bsearch (const void* key, const void* base,
               size_t num, size_t size,
               int (*compar)(const void*,const void*));
*/

