
#include "hashtable.h"

void hashtable_sethashfunc(hashtable * t, unsigned int (*h)(void *)) {
    t->_hash = h;
}

void hashtable_setcompfunc(hashtable * t, int (*c)(void *, void *)) {
    t->_comp = c;
}

void hashtable_setfreefunc(hashtable * t, void (*f)(void *, void *)) {
    t->_free = f;
}

hashtable * hashtable_init(const unsigned int tblsize) {
    hashtable * t;

    t = (hashtable *) malloc(sizeof(hashtable));
    t->_table = (hashtable_value *) calloc(tblsize, sizeof(hashtable_value));
    t->size = 0;
    t->table_size = tblsize;

    return t;
}

void hashtable_free(hashtable * t) {

    hashtable_value * c;
    hashtable_value * p;

    hashtable_value * cur = t->_table;
    unsigned int i = 0;
    do {
        if (cur->key != NULL) { // not empty
            // release linked list of the entry
            c = cur->next;
            while (c != NULL) {
                p = c;
                c = c->next;
                t->_free(p->key, p->data);
                free(p);
            }
        }
        // move to next entry in the table
        cur++;
        i++;
    } while (i < t->table_size);

    free(t->_table);
    free(t);

}

void hashtable_put(hashtable * t, void * k, void * v) {

    unsigned int idx = t->_hash(k) % t->table_size;
    hashtable_value * cur = &t->_table[idx];

    if (cur->key == NULL) { // empty entry

        cur->key = k;
        cur->data = v;
        cur->next = NULL;
        cur->last = NULL;
        // last is NULL when there is nothing appended

    } else {

        // create a key-value pair
        hashtable_value * val = (hashtable_value *) malloc(sizeof(hashtable_value));
        val->key = k;
        val->data = v;
        val->next = NULL;
        val->last = NULL;

        if (cur->last == NULL) {
            // not empty & nothing appended
            // so, it's the second
            cur->next = val;
            cur->last = val;
        } else {
            // for simplicity, there is no duplication prevention
            // directly append to the last
            cur->last->next = val;
            cur->last = val;
        }

    }

    // count key-value pairs
    t->size++;

}

void * hashtable_get(hashtable * t, void * k) {

    unsigned int idx = t->_hash(k) % t->table_size;
    hashtable_value * cur = &t->_table[idx];

    if (cur->key != NULL) {
        // for a non-empty bucket,
        // search for a pair with the exact key
        do {
            if (t->_comp(cur->key, k))
                return cur->data; // found
            cur = cur->next;
        } while (cur != NULL);
    }

    return NULL; // not found

}
