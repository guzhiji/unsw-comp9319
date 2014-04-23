
#include "bwtsearch.h"

#include "exarray.h"
#include "chargroup.h"
#include "bwttext.h"
#include <stdio.h>
#include <stdlib.h>

fpos_range * search_range(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    r = (fpos_range *) malloc(sizeof(fpos_range));

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    if (c == NULL) return NULL;
    r->first = c->smaller_symbols;
    r->last = r->first + c->info->frequency - 1;

    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) return NULL;
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
    }

    if (r->first <= r->last) return r;
    return NULL;

}

//TODO use bsearch
unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    chargroup_list * list = chargroup_list_get(t, c);
    exarray_cursor * cur = NULL;
    bwtindex_chargroup * cg;

    while ((cur = exarray_next(list->groups, cur)) != NULL) {
        cg = (bwtindex_chargroup *) cur->data;
        if (pos >= list->position_base + cg->offset)
            return cg->occ_before;
    }
    return 0;

}
