
#include "hashtable.h"

unsigned int (*_hashtable_hash)(void * k);

void hashtable_sethashfunc(unsigned int (*h)(void *)) {
    _hashtable_hash = h;
}

int (*_hashtable_comp)(void * k1, void * k2);

void hashtable_setcompfunc(int (*c)(void *, void *)) {
    _hashtable_comp = c;
}

hashtable * hashtable_init() {
    hashtable * t;
    int i;

    t = (hashtable *) malloc(sizeof(hashtable));
    t->_table = (hashtable_value **) malloc(sizeof(hashtable_value *) * _TABLE_SIZE_);
    t->size = 0;
    for (i = 0; i < _TABLE_SIZE_; i++)
        t->_table[i] = NULL;

    return t;
}

void hashtable_free(hashtable * t) {

    int i;

    for (i = 0; i < _TABLE_SIZE_; i++) {
        if (t->_table[i] != NULL) {
            hashtable_value * cur;
            hashtable_value * p;
            cur = t->_table[i];
            while (cur->next != NULL) {
                p = cur;
                cur = cur->next;
                if (p->key == p->data) {
                    free(p->data);
                } else {
                    free(p->key);
                    free(p->data);
                }
                free(p);
            }
            free(cur);
        }
    }
    free(t->_table);
    free(t);

}

unsigned int hashtable_hash(void * k) {
    return _hashtable_hash(k) % _TABLE_SIZE_;
}

void hashtable_put(hashtable * t, void * k, void * v) {

    hashtable_value * val = (hashtable_value *) malloc(sizeof(hashtable_value));
    int idx = hashtable_hash(k);
    val->key = k;
    val->data = v;
    val->next = NULL;

    if (t->_table[idx] == NULL) {
        t->_table[idx] = val;
    } else {
        hashtable_value * cur;
        cur = t->_table[idx];
        while (cur->next != NULL)
            cur = cur->next; // for simplicity, there is no duplication prevention
        cur->next = val;
    }
    t->size++;

}

void * hashtable_get(hashtable * t, void * k) {

    int idx = hashtable_hash(k);
    if (t->_table[idx] == NULL) {
        return NULL; // not found
    } else {
        hashtable_value * cur;
        cur = t->_table[idx];
        do {
            if (_hashtable_comp(cur->key, k))
                return cur->data;
            cur = cur->next;
        } while (cur != NULL);
        return NULL; // not found
    }

}

