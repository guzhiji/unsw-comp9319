
#include "hashtable.h"

/**
 * internal custom hash function pointer
 */
unsigned int (*_hashtable_hash)(void * k);

void hashtable_sethashfunc(unsigned int (*h)(void *)) {
    _hashtable_hash = h;
}

/**
 * internal custom compare function pointer
 */
int (*_hashtable_comp)(void * k1, void * k2);

void hashtable_setcompfunc(int (*c)(void *, void *)) {
    _hashtable_comp = c;
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
                if (p->key == p->data) {
                    // key and data may point to the same
                    // block of memory
                    free(p->data);
                } else {
                    free(p->key);
                    free(p->data);
                }
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

unsigned int hashtable_hash(void * k, const unsigned int tblsize) {
    return _hashtable_hash(k) % tblsize;
}

void hashtable_put(hashtable * t, void * k, void * v) {

    int idx = hashtable_hash(k, t->table_size);
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

    int idx = hashtable_hash(k, t->table_size);
    hashtable_value * cur = &t->_table[idx];

    if (cur->key != NULL) {
        // for a non-empty bucket,
        // search for a pair with the exact key
        do {
            if (_hashtable_comp(cur->key, k))
                return cur->data; // found
            cur = cur->next;
        } while (cur != NULL);
    }

    return NULL; // not found

}
