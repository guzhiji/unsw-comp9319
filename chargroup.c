
#include "chargroup.h"

/**
 * read a complete group list for a character
 */
chargroup_list * chargroup_list_read(FILE * fp, unsigned char c) {
    // fread
}

chargroup_list * chargroup_list_init(unsigned char c) {
    chargroup_list * list = (chargroup_list *) malloc(sizeof(chargroup_list));
    list->groups = exarray_init(CHARGROUP_LIST_SIZE, CHARGROUP_LIST_SIZE, sizeof(chargroup));
    list->c = c;
    return list;
}

void chargroup_list_write(chargroup_list * cgl, FILE * fp) {
    chargroup_list * cur;

    cur = cgl;
    while (cur != NULL) {
        fwrite(cur->info, sizeof(chargroup_listinfo), 1, fp);
        fwrite(cur->groups, sizeof(chargroup), cur->info->length, fp);
        cur = cur->next;
    }
}

unsigned int chargroup_list_size(chargroup_list * l) {
}

unsigned int chargroup_list_free(chargroup_list * l) {
    // never release the head, it's in bwttext
    // return total number of groups
}

chargroup_list * chargroup_list_get(bwttext * t, unsigned char c) {
    unsigned int i;
    chargroup_list * l = t->chargroup_list_hash[(unsigned int) c];

    if (l == NULL || l->info == NULL) { // not loaded yet

        // release the least frequent ones
        while (t->chargroup_num >= CHARGROUP_NUM_THRESHOLD) {
            t->chargroup_num -= chargroup_list_free(&t->chargroup_list_sorted[--t->chargroup_list_num]);
        }

        // load the chargroup list for c from index file
        l = chargroup_list_read(t->ifp, c);
        t->chargroup_num += chargroup_list_size(l);
        t->chargroup_list_hash[(unsigned int) c] = l;

        // insert the new list by frequency
        i = t->chargroup_list_num++; // point to the one after the last and increment count
        while (1) {
            if (i >= 1 && t->chargroup_list_sorted[i - 1]->cp->frequency < l->cp->frequency) {
                // push i-1 to i
                t->chargroup_list_sorted[i] = t->chargroup_list_sorted[--i];
            } else {
                // insert at i
                t->chargroup_list_sorted[i] = l;
                break;
            }
        }

    }

    return l;
}

void chargroup_list_add(bwttext * t, unsigned char c, chargroup * cg) {

    chargroup_list * list = t->chargroup_list_hash[(unsigned int) c];
    if (list == NULL) {
        list = t->chargroup_list_hash[(unsigned int) c] = chargroup_list_init(c);
        t->chargroup_list_num++;
    }
    exarray_add(list->groups, cg);
    t->chargroup_num++;

    if (t->chargroup_num >= CHARGROUP_NUM_THRESHOLD) {
        // write to disk
        int i;
        for (i = 0; i< 256; i++) {
            list = t->chargroup_list_hash[i];
            if (list != NULL) {
                exarray_save(t->ifp, list->groups);
                exarray_free(list->groups);
                t->chargroup_list_hash[i] = NULL;
            }
        }
        t->chargroup_list_num = 0;
        t->chargroup_num = 0;
    }
}

void chargroup_add(bwttext * t, FILE * fp, chargroup * cg, unsigned char c) {
}

chargroup_list * chargroup_list_load(FILE * fp) {

}

/**
 * order by character frequency;
 * less frequent ones are closer to the end
 */
int char_cmp_by_freq(const void * c1, const void * c2) {
    character * ch1 = (character *) c1;
    character * ch2 = (character *) c2;
    if (ch1->frequency > ch2->frequency)
        return -1;
    else if (ch1->frequency < ch2->frequency)
        return 1;
    else
        return 0;
}

/**
 * order by character frequency;
 * less frequent ones are closer to the end
 */
int chargroup_list_cmp(const void * c1, const void * c2) {
    chargroup_list * cgl1 = (chargroup_list *) c1;
    chargroup_list * cgl2 = (chargroup_list *) c2;
    if (cgl1->cp->frequency > cgl2->cp->frequency)
        return -1;
    else if (cgl1->cp->frequency < cgl2->cp->frequency)
        return 1;
    else
        return 0;
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

void chargroup_write(chargroup * cg, char c) {
    if (cg->c == c) {
        cg->data.size++;
    } else {
        // write cg->data
        cg->data.start = cg->data.size;
        cg->data.size = 1;
        cg->c = c;
    }
}

