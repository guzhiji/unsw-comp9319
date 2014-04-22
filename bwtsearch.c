
#include "bwtsearch.h"

fpos_range * search_range(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    r = malloc(sizeof(fpos_range));

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    r->first = c->smaller_symbols;
    r->last = r->first + c->info->frequency - 1;

    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
    }

    if (r->first <= r->last) return r;
    return NULL;

}

//TODO use bsearch
unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    chargroup_list * list = chargroup_list_get(t, c);
    chargroup * g = list->groups;
    unsigned long o = 0;
    unsigned int i;
    for (i = 0; i < list->info->length; i++) {
        if (pos >= list->info->start + g->start) {
            o += pos - list->info->start - g->start;
            break;
        }
        o += g->size;
        g++;
    }
    return o;
}

