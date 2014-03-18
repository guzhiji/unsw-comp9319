
#include "hashtable.h"

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

    // printf("size=%d\n", t->size);

    for (i = 0; i < _TABLE_SIZE_; i++) {
        if (t->_table[i] != NULL) {
            hashtable_value * cur;
            hashtable_value * p;
            cur = t->_table[i];
            while (cur->next != NULL) {
                // if (cur->key != 0)
                //     printf("i=%d,k=%d,v=%d\n", i, cur->key, cur->data);
                p = cur;
                cur = cur->next;
                free(p);
            }
            // if (cur->key != 0)
            //     printf("i=%d,k=%d,v=%d\n", i, cur->key, cur->data);
            free(cur);
        }
    }
    free(t->_table);
    free(t);
}

int hashtable_hash(unsigned int k) {
    // return abs(k) % _TABLE_SIZE_;
    return k % _TABLE_SIZE_;
}

void hashtable_put(hashtable * t, int k, void * v) {

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
            cur = cur->next;
        cur->next = val;
    }
    t->size++;

}

void * hashtable_get(hashtable * t, int k) {
    int idx = hashtable_hash(k);
    if (t->_table[idx] == NULL) {
        return NULL; // not found
    } else {
        hashtable_value * cur;
        cur = t->_table[idx];
        do {
            if (cur->key == k)
                return cur->data;
            cur = cur->next;
        } while (cur != NULL);
        return NULL; // not found
    }
}
