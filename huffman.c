
#include "huffman.h"

void huffman_pq_push(huffman_pqueue * pq, huffman_node * n) {

    if (pq->head == NULL) {
        pq->head = n;
    } else {

        huffman_node * pn; // previous node
        huffman_node * cn; // current node

        pn = pq->head;
        cn = pq->head->higher;
        while (cn != NULL && n->freq > cn->freq) {
            pn = cn;
            cn = cn->higher;
        }

        pn->higher = n;
        n->higher = cn;

    }
}

huffman_node * huffman_pq_pop(huffman_pqueue * pq) {

    huffman_node * n;

    if (pq->head == NULL)
        return NULL;

    n = pq->head;
    pq->head = pq->head->higher;
    n->higher = NULL;

    return n;

}

huffman_node * _getallsymbols(FILE * fp, hashtable * stats) {

    int c; // character
    huffman_node * cn; // current node
    huffman_node * fn; // first node
    huffman_node * pn; // previous node

    fn = NULL;
    pn = NULL;

    while ((c = fgetc(fp)) != EOF) {

        cn = (huffman_node *) hashtable_get(stats, c);
        if (cn != NULL) { // a known char
            cn->freq++;

        } else { // an unknown char

            cn = (huffman_node *) malloc(sizeof(huffman_node));
            cn->symbol = c;
            cn->freq = 1;
            cn->next = NULL;
            cn->parent = NULL;
            cn->higher = NULL;

            if (pn == NULL) {
                fn = cn;
                pn = cn;
            } else {
                pn->next = cn;
                pn = cn;
            }

            hashtable_put(stats, c, cn);
        }

    }

    return fn;
}

void _constructtree(huffman_node * firstnode) {

    huffman_node * n1;
    huffman_node * n2;
    huffman_node * pn;
    huffman_pqueue * pqueue;

    pqueue = (huffman_pqueue *) malloc(sizeof(huffman_pqueue));
    pqueue->head = NULL;

    n1 = firstnode;
    while (n1 != NULL) {
        n2 = n1->next; // n2 is used as an intermediate node
        huffman_pq_push(pqueue, n1); // this changes n1->next
        n1 = n2;
    }

    n1 = huffman_pq_pop(pqueue);
    n2 = huffman_pq_pop(pqueue);
    while (n2 != NULL && n1 != NULL) {

        pn = (huffman_node *) malloc(sizeof(huffman_node));
        pn->symbol = 0;
        pn->next = NULL;
        pn->parent = NULL;
        pn->higher = NULL;
        pn->freq = n1->freq + n2->freq;

        n1->parent = pn;
        n2->parent = pn;

        huffman_pq_push(pqueue, pn);

        n1 = huffman_pq_pop(pqueue);
        n2 = huffman_pq_pop(pqueue);
    }

    free(pqueue);
/*
    if (n1 == NULL) {
        // nothing in the tree

    } else {
        // n1 != NULL
        // but
        // n2 == NULL
        // n1 is the last element, the root of the tree

    }
*/

}

int _csize(huffman_node * first) {

    int s, l;
    huffman_node * cn;
    huffman_node * n;

    s = 0;
    cn = first;
    while (cn != NULL) {

        l = 0;
        n = cn->parent;
        while (n != NULL) {
            l++;
            n = n->parent;
        }

        s += l * cn->freq;

        cn = cn->next;

    }

    return s;

}

int huffman_csize(FILE * fp) {

    int s;
    huffman_node * first;
    hashtable * ht = hashtable_init();
    first = _getallsymbols(fp, ht);
    _constructtree(first);
    s = _csize(first);
    hashtable_free(ht);

    if (s % 8 > 0)
        return s / 8 + 1;
    return s / 8;

}
