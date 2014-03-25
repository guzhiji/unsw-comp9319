
#include "huffman.h"

void huffman_pq_push(huffman_pqueue * pq, huffman_node * n) {

    huffman_node * pn; // previous node
    huffman_node * cn; // current node

    pn = NULL; // head is supposed to be the lowest
    cn = pq->head;
    while (cn != NULL && n->freq > cn->freq) {
        pn = cn;
        cn = cn->higher;
    }
    // if cn is NULL, pn is the last, so n is appended at the end
    // if n is less frequent than cn, n is before cn,
    // and of course after pn
    // but, if pn is NULL, n is the first for the queue can be empty,
    // or n can be less frequent than the first

    if (pn == NULL) {
        // n lower than head, or head can be nothing
        n->higher = pq->head;
        pq->head = n;
    } else {
        pn->higher = n; // n is after pn
        n->higher = cn; // n is before cn
        // so n is inserted between cn and pn, if cn is not NULL
    }

}

huffman_node * huffman_pq_pop(huffman_pqueue * pq) {

    huffman_node * n;

    if (pq->head == NULL)
        return NULL;

    // head is the least frequent, so get it
    n = pq->head;
    pq->head = pq->head->higher;
    n->higher = NULL;

    return n;

}

/**
 * read all symbols, link them together, and count frequencies.
 *
 * @param FILE * fp             file pointer
 * @param hashtable * stats     frequencies of all symbols
 * @return huffman_node *       the first node of all nodes/symbols
 */
huffman_node * _getallsymbols(FILE * fp, hashtable * stats) {

    int c; // character
    huffman_node * cn; // current node
    huffman_node * fn; // first node
    huffman_node * pn; // previous node

    fn = NULL;
    pn = NULL;

    while ((c = fgetc(fp)) != EOF) {

        cn = (huffman_node *) hashtable_get(stats, &c);
        if (cn != NULL) { // a known char

            cn->freq++; // count frequency for the character

        } else { // an unknown char

            // will be used for the hashtable, stats
            int * symbol = (int *) malloc(sizeof(int));
            *symbol = c;

            // create a Huffman node the new character
            cn = (huffman_node *) malloc(sizeof(huffman_node));
            cn->symbol = c;
            cn->freq = 1; // the first time encountering the character
            cn->next = NULL;
            cn->parent = NULL;
            cn->higher = NULL;

            // link Huffman nodes together
            if (pn == NULL) // no previous node
                fn = cn; // it's the first node
            else
                pn->next = cn; // the previous's next is the current
            pn = cn; // for the next, the previous is the current

            // persist the symbol in the hashtable storage
            hashtable_put(stats, symbol, cn);
        }

    }

    // return the first node so as to facilitate further enumeration
    return fn;

}

/**
 * construct a Huffman tree.
 *
 * inputs a chain of symbol nodes and outputs a chain of parent nodes,
 * and between them, a tree hierarchy is also constructed.
 *
 * @param huffman_node * firstnode      the first node (leaf node) from which enumeration starts
 * @return huffman_node *               the first parent node (rather than leaf nodes)
 */
huffman_node * _constructtree(huffman_node * firstnode) {

    // for tree construction
    huffman_node * n1; // node 1
    huffman_node * n2; // node 2
    huffman_node * pn; // parent node
    huffman_pqueue * pqueue; // priority queue

    // for enumeration and resource releasing purposes
    huffman_node * fpn; // first parent node
    huffman_node * ppn; // previous parent node

    // create a priority queue
    pqueue = (huffman_pqueue *) malloc(sizeof(huffman_pqueue));
    pqueue->head = NULL;

    // enumerate all symbols and push them into the queue,
    // so that they are sorted
    n1 = firstnode;
    while (n1 != NULL) {
        n2 = n1->next; // n2 is used as an intermediate node
        huffman_pq_push(pqueue, n1); // this changes n1->next
        n1 = n2;
    }

    fpn = NULL;
    ppn = NULL;

    // take the least frequent two nodes in the queue
    n1 = huffman_pq_pop(pqueue);
    n2 = huffman_pq_pop(pqueue);
    while (n2 != NULL && n1 != NULL) {

        // create a parent node for the least frequent two nodes in the queue
        pn = (huffman_node *) malloc(sizeof(huffman_node));
        pn->symbol = 0; // a parent node does not represent a single symbol
        pn->next = NULL;
        pn->parent = NULL;
        pn->higher = NULL;
        pn->freq = n1->freq + n2->freq; // sum children's frequencies up

        n1->parent = pn;
        n2->parent = pn;

        // link all parent nodes together
        if (ppn == NULL) // no previous parent nodes
            fpn = pn; // it's the first parent node
        else
            ppn->next = pn; // the previous's next is the current parent node
        ppn = pn; // for the next parent node, it's the previous parent node

        // push the parent node into the queue,
        // instead of the two child nodes
        huffman_pq_push(pqueue, pn);

        // take the least frequent two nodes in the queue
        // for the next iteration
        n1 = huffman_pq_pop(pqueue);
        n2 = huffman_pq_pop(pqueue);
    }
    // eventually, all nodes are combined to a root node
    // and the tree is constructed

    free(pqueue);

    // return the first parent node
    return fpn;

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

/**
 * calculate Huffman compression size in bits.
 *
 * @param huffman_node * first      the first node from which enumeration starts
 * @return int      bits
 */
int _csize(huffman_node * first) {

    int s, l;
    huffman_node * cn;
    huffman_node * n;

    s = 0;

    // enumerate symbols: the leaf nodes
    cn = first;
    while (cn != NULL) {

        // length of Huffman code:
        // count the number of parent levels
        l = 0;
        n = cn->parent;
        while (n != NULL) {
            l++;
            n = n->parent;
        }

        // size of all the same symbol
        // in Huffman code
        s += l * cn->freq;

        cn = cn->next;

    }

    return s;

}

/**
 * custom function to accept integer for hashing
 *
 * @param int *             pointer to an int key
 * @return unsigned int     value of the int as a hash key
 */
unsigned int hashtable_hash_int(void * k) {
    return * (int *) k;
}

/**
 * custom function to accept integers for comparison
 *
 * @param int *     pointer to an int key
 * @param int *     pointer to an int key
 * @return int      1 for equal; 0 for inequal
 */
int hashtable_comp_int(void * k1, void * k2) {
    return (* (int *) k1) == (* (int *) k2);
}

int huffman_csize(FILE * fp) {

    int s;
    huffman_node * firstsymbol;
    huffman_node * firstparent;
    huffman_node * tmp;
    hashtable * ht = hashtable_init();

    // setup hashtable custom functions
    hashtable_setcompfunc(hashtable_comp_int);
    hashtable_sethashfunc(hashtable_hash_int);

    firstsymbol = _getallsymbols(fp, ht);
    if (firstsymbol != NULL) {
        firstparent = _constructtree(firstsymbol);
        s = _csize(firstsymbol);
    } else {
        firstparent = NULL;
        s = 0;
    }

    // release parent nodes
    while (firstparent != NULL) {
        tmp = firstparent;
        firstparent = firstparent->next;
        free(tmp);
    }

    // release leaf nodes/symbol nodes, 
    // since they are in the hashtable storage
    hashtable_free(ht);

    // bits to bytes
    if (s % 8 > 0)
        return s / 8 + 1;
    return s / 8;

}
