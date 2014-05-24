
#include "plset.h"

#include "strbuf.h"
#include <stdlib.h>

plset * plset_init() {
    plset * s = (plset *) malloc(sizeof(plset));
    s->arr = (pline *) malloc(sizeof(pline) * 32);
    s->max = 32;
    s->len = 0;
    return s;
}

int plset_contains(plset * s, unsigned long pos) {
    unsigned long i, l;
    l = s->len;
    for (i = 0; i < l; i++)
        if (s->arr[i].pos == pos)
            return 1;
    return 0;
}

void plset_put(plset * s, unsigned long pos, strbuf * sb1, strbuf * sb2) {

    //if (plset_contains(s, pos)) return;

    if (s->len == s->max) {

        pline * la = (pline *) realloc(s->arr, sizeof(pline) * (s->max + 32));

        if (la == NULL) return;

        s->max += 32;
        s->arr = la;
    }

    pline * l = &s->arr[s->len++];
    l->pos = pos;
    l->sb1 = sb1;
    l->sb2 = sb2;

}

int _cmp_line_by_p(const void * l1, const void * l2) {
    return ((pline *) l1)->pos - ((pline *) l2)->pos;
}

void plset_sort(plset * s) {
    qsort(s->arr, s->len, sizeof (pline), _cmp_line_by_p);
}

void plset_print(plset * s, FILE * fout) {
    int i;
    pline * l = s->arr;
    for (i = 0; i < s->len; i++) {
        strbuf_dump_rev(l->sb1, fout);
        strbuf_dump(l->sb2, fout);
        l++;
    }
}

void plset_free(plset * s) {
    int i;
    pline * l = s->arr;
    for (i = 0; i < s->len; i++) {
        strbuf_free(l->sb1);
        strbuf_free(l->sb2);
        l++;
    }
    free(s->arr);
    free(s);
}

