
typedef struct {
    unsigned long first;
    unsigned long last;
} fpos_range;

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
    r->last = r->first + c->frequency - 1;

    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
    }

    if (r->first <= r->last) return r;
    return NULL;

}
