
#include "exarray.h"
//#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

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

unsigned long exarray_free(exarray * a) {
    unsigned int i;
    unsigned long l = 0;
    exarray_node * t, * n = a->head;
    while (n != NULL) {
        l += n->size;

        t = n;
        n = n->next;
        // TODO specialized free function

        for (i = 0; i < t->size; i++)
            free(t->arr[i]);

        free(t->arr);
        free(t);

    }
    free(a);
    return l;
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
        if (a->head->size > 0) {
            a->tail->next = newarray->head;
            a->tail = newarray->tail;
        } else { // TODO it's just a tweak and should be changed
            free(a->head->arr);
            free(a->head);
            a->head = newarray->head;
            a->tail = newarray->tail;
        }
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
        cur->data = a->head->arr[0];
        return cur;
    } else if (cur->position + 1 < cur->node->size) {
        // the next element is still in the same node
        cur->data = cur->node->arr[++cur->position];
        return cur;
    } else if (/* cur->node != a->tail && */ cur->node->next != NULL) {
        // there are nodes afterwards
        cur->node = cur->node->next;
        cur->position = 0;
        cur->data = cur->node->arr[0];
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
    
    n = a->head;
    while (n != NULL) {
        nc++;
        n = n->next;
    }
    fwrite(&nc, sizeof(unsigned int), 1, f);

    n = a->head;
    while (n != NULL) {
        fwrite(&n->size, sizeof(unsigned int), 1, f);
        for (i = 0; i < n->size; i++)
            fwrite(n->arr[i], a->unit_size, 1, f);
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
    if (nc < 1) {
        fprintf(stderr, "error");
        exit(1);
    }
    for (i = 0; i < nc; i++) {
        l = 0;
        fread(&l, sizeof(unsigned int), 1, f);

        n = (exarray_node *) malloc(sizeof(exarray_node));
        n->size = n->capacity = l;
        n->arr = malloc(sizeof(void *) * l);
        for (j = 0; j < l; j++) {
            n->arr[j] = malloc(unit_size);
            fread(n->arr[j], unit_size, 1, f);
        }

        n->next = NULL;
        if (p == NULL) // no previous, so it's the first
            a->head = n;
        else // the previous's next is the current node
            p->next = n;
        p = n;
    }
    //a->tail = n;
    a->tail = p;

    return a;
}

/*
TODO
void* bsearch (const void* key, const void* base,
               size_t num, size_t size,
               int (*compar)(const void*,const void*));


int main() {
    int *aa, *bb, *cc,*dd, *ee, *ff, *gg;
    FILE * fp;
    exarray * a = exarray_init(10, 5, sizeof(int));
    exarray_cursor * cur = NULL;
    
    
    aa = (int *) malloc(sizeof(int));
    *aa = 435;
    bb = (int *) malloc(sizeof(int));
    *bb = 56865;
    cc = (int *) malloc(sizeof(int));
    *cc = 1234;
    dd = (int *) malloc(sizeof(int));
    *dd = 67;
    ee = (int *) malloc(sizeof(int));
    *ee = 234;
    ff = (int *) malloc(sizeof(int));
    *ff = 6574;
    gg = (int *) malloc(sizeof(int));
    *gg = 2357;
    exarray_add(a, aa);
    exarray_add(a, bb);
    exarray_add(a, cc);
    exarray_add(a, dd);
    exarray_add(a, ee);
    exarray_add(a, ff);
    exarray_add(a, gg);

    fp = fopen("test1.idx", "wb");
    exarray_save(a, fp);
    fclose(fp);

    exarray_free(a);

    fp = fopen("test1.idx", "rb");
    a = exarray_load(fp, 5, sizeof(int));
    fclose(fp);

    fp = fopen("test2.idx", "wb");
    exarray_save(a, fp);
    fclose(fp);

    while ((cur = exarray_next(a, cur)) != NULL) {
        printf("%d\n", *(int *)cur->data);
    }
    
    
    exarray_free(a);

    return 0;
}


*/
