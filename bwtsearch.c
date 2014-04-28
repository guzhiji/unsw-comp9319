
#include "bwtsearch.h"

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
            printf("(start: %lu, occ: %lu)\n", cg->offset, cg->occ_before);
        }

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
        
        bwtindex_chargrouplist_load(t, ch);
        cur = NULL;
        while ((cur = exarray_next(ch->grouplist->groups, cur)) != NULL) {
            bwtindex_chargroup * cg = (bwtindex_chargroup *) cur->data;
            
        }

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
    //printf("%lu, %lu\n", r->first, r->last);
    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) return NULL;
        //printf("[%lu, %lu, %lu]\n", c->smaller_symbols, occ(t, x, r->first), occ(t, x, r->last + 1) - 1);
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
        //printf("%lu, %lu\n\n", r->first, r->last);
    }

    if (r->first <= r->last) return r;
    return NULL;

}

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    bwtblock blk;

    if (!bwtblock_find(t, pos, c, &blk)) // weird if so
        return 0;

    if (blk.pl > 0) {// a pure block
        if (blk.c == c) // all c in the block
            return blk.occ + pos - blk.pos;//get by position calculation
        else // no c in the block
            return blk.occ; // occ at the beginning
    }

    // TODO pos is ftell or position of char in bwttext
    // TODO scan bwttext for c from blk.pos within |blk.pl|

    return 0;
}

void decode_backword(bwttext * t) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end_position;
    //int i=0;
    do {
        //printf("\ni: %d, pos: %lu, ", i, p);
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        //printf("char=%d, ", c);
        putchar(c);//if (i==5) break;
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: %d\n", c);
            break; // error
        }
        p = ch->smaller_symbols + occ(t, c, p);//i++;
    } while (p != t->end_position);
    //printf("\n");
    
}

