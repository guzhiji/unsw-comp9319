
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
            printf("(start: %lu, occ: %lu)\n", cg->start, cg->occ_before);
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

    r = (fpos_range *) malloc(sizeof (fpos_range));

    pp = l - 1;
    x = p[pp];

    c = t->char_hash[(unsigned int) x];
    if (c == NULL) return NULL;
    r->first = c->smaller_symbols;
    r->last = r->first + c->info->frequency - 1;
    //printf("%c: %lu, %lu\n", c->info->c, r->first, r->last);
    while (r->first <= r->last && pp > 0) {
        x = p[--pp];
        c = t->char_hash[(unsigned int) x];
        if (c == NULL) return NULL;
        //printf("[%lu, %lu, %lu]\n", c->smaller_symbols, occ(t, x, r->first), occ(t, x, r->last + 1) - 1);
        r->first = c->smaller_symbols + occ(t, x, r->first);
        r->last = c->smaller_symbols + occ(t, x, r->last + 1) - 1;
        //printf("%c: %lu, %lu\n", c->info->c, r->first, r->last);
    }

    if (r->first <= r->last) return r;
    return NULL;

}

//TODO use bsearch

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    chargroup_list * list = chargroup_list_get(t, c);
    exarray_cursor * cur = NULL;
    bwtindex_chargroup * cg, * pcg = NULL;

    while ((cur = exarray_next(list->groups, cur)) != NULL) {
        cg = (bwtindex_chargroup *) cur->data;
        if (pos < cg->start) {
            if (pcg == NULL) return 0;
            if (pos < pcg->start + pcg->size)
                return pcg->occ_before + pos - pcg->start;
            return pcg->occ_before + pcg->size;
        }
        pcg = cg;
    }
    if (pcg == NULL) return 0;
    if (pos < pcg->start + pcg->size)
        return pcg->occ_before + pos - pcg->start;
    return pcg->occ_before + pcg->size;

}

unsigned char fpos_char(bwttext * t, unsigned long fpos) {
    int i;
    unsigned char p = 0;
    character * c;
    for (i = 0; i < 256; i++) {
        c = t->char_hash[i];
        if (c == NULL) continue;
        if (c->smaller_symbols > fpos) break;
        p = c->info->c;
    }
    return p;
}

void decode_backword(bwttext * t) {
    unsigned char c;
    character * ch;
    unsigned long p = t->end_position;

    do {
        //c = fpos_char(t, p);
        fseek(t->fp, p + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        putchar(c);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: %d\n", c);
            break; // error
        }
        p = ch->smaller_symbols + occ(t, c, p); //i++;
    } while (p != t->end_position);
    //printf("\n");

}

// pos_prev is a fpos; it gets its previous char at it's lpos

void decode_backward_until(bwttext * t, unsigned long pos_prev, unsigned char until) {
    unsigned char c;
    character * ch;
    do {
        fseek(t->fp, pos_prev + 4, SEEK_SET);
        fread(&c, sizeof (unsigned char), 1, t->fp);
        putchar(c);
        ch = t->char_hash[(unsigned int) c];
        if (ch == NULL) {
            fprintf(stderr, "\nerror: %d\n", c);
            break; // error
        }
        pos_prev = ch->smaller_symbols + occ(t, c, pos_prev);
    } while (pos_prev != t->end_position && until != c);

}

unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ) {
    int tc;
    unsigned long n = 0, p = 0;
    fseek(t->fp, 4, SEEK_SET);
    while ((tc = fgetc(t->fp)) != EOF) {
        if (tc == c && n++ == occ)
            return p;
        p++;
    }
    return p;
}

void decode_forward_until(bwttext * t, unsigned long pos, unsigned char until) {
    unsigned char c;
    unsigned long occ, p = pos;
    character * ch;
    //printf("decode forward: pos %lu\n", pos);
    while (p != t->end_position) {
        c = fpos_char(t, p); //printf("\n- %d: ", c);
        putchar(c);
        if (c == until) return;
        ch = t->char_hash[(unsigned int) c];
        occ = p - ch->smaller_symbols; // occ for the next char
        p = lpos(t, c, occ);
    }
    fseek(t->fp, 4 + t->end_position, SEEK_SET);
    c = fgetc(t->fp);
    putchar(c);

}
