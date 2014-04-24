
#include "bwtsearch.h"

#include "exarray.h"
#include "chargroup.h"
#include "bwttext.h"
#include <stdio.h>
#include <stdlib.h>

void dump_chartable(bwttext * t) {
    int i;
    character * ch;
    printf("==================\n");
    printf("dump_chartable:\n");
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        printf("%c (%d): ss=%lu, freq=%lu\n", ch->info->c, ch->info->c, ch->smaller_symbols, ch->info->frequency);
    }
    printf("==================\n");
}

void dump_occ(bwttext * t) {
    int i;
    character * ch;
    exarray_cursor * cur;
    printf("==================\n");
    printf("dump_occ:\n");
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        printf("%c (%d):\n", ch->info->c, ch->info->c);
        if (ch->grouplist == NULL) continue;
        cur = NULL;
        while ((cur = exarray_next(ch->grouplist->groups, cur)) != NULL) {
            bwtindex_chargroup * cg = (bwtindex_chargroup *) cur->data;
            printf("%lu (%lu, %d): %lu\n", ch->grouplist->position_base + cg->offset, ch->grouplist->position_base, cg->offset, cg->occ_before);
        }
        printf("last size = %d\n", ch->grouplist->last_chargroup_size);

    }
    printf("==================\n");
}

void dump_pos(bwttext * t) {
    int i;
    character * ch;
    exarray_cursor * cur;
    printf("==================\n");
    printf("dump_pos:\n");
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        printf("%c (%d):", ch->info->c, ch->info->c);
        if (ch->chargroup_list_positions == NULL) continue;
        cur = NULL;
        while ((cur = exarray_next(ch->chargroup_list_positions, cur)) != NULL) {
            unsigned long * p = (unsigned long *) cur->data;
            printf("%lu;", *p);
        }
        printf("\n");

    }
    printf("==================\n");
}

fpos_range * search_range(bwttext * t, unsigned char * p, unsigned int l) {

    fpos_range * r;
    character * c;
    unsigned int pp;
    unsigned char x;

    //dump_chartable(t);
    //dump_pos(t);
    
    r = (fpos_range *) malloc(sizeof(fpos_range));

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    if (c == NULL) return NULL;
    r->first = c->smaller_symbols;
    r->last = r->first + c->info->frequency - 1;
    printf("%lu, %lu\n", r->first, r->last);
    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) return NULL;
        printf("[%lu, %lu, %lu]\n", c->smaller_symbols, occ(t, x, r->first), occ(t, x, r->last + 1) - 1);
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
        printf("%lu, %lu\n\n", r->first, r->last);
    }

    if (r->first <= r->last) return r;
    return NULL;

}

//TODO use bsearch
unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    chargroup_list * list = chargroup_list_get(t, c);
    exarray_cursor * cur = NULL;
    bwtindex_chargroup * cg, * pcg = NULL;
    //printf("occ> -------------------\n");
    while ((cur = exarray_next(list->groups, cur)) != NULL) {
        cg = (bwtindex_chargroup *) cur->data;
        //printf("occ> pos: cur %lu, begin %lu\n", pos, list->position_base + cg->offset);
        //printf("occ> occ: cur begin %lu, prev begin %lu\n", cg->occ_before, pcg==NULL ? 0 : pcg->occ_before);
        if (pos < list->position_base + cg->offset) {
            return pcg == NULL ? 0 : pcg->occ_before + pos - (list->position_base + pcg->offset);
        }
        pcg = cg;
    }
    return list->last_chargroup_size;

}

void decode(bwttext * t) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end_position;

    do {
        printf("\npos: %lu, ", p);
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        printf("char=%d, ", c);
        putchar(c);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            printf("\nerror: %d\n", c);
            break; // error
        }
        p = ch->smaller_symbols + occ(t, ch->info->c, p);
    } while (p != t->end_position);
    
}
