
#include "chartable.h"

#include <stdio.h>
#include <stdlib.h>

void chartable_inithash(bwttext * t) {
    int i;
    for (i = 0; i < 256; i++)
        t->char_hash[i] = NULL;
}

void chartable_init(bwttext * t) {

    //t->char_table_pos = 
    unsigned long p = t->occ_infreq_pos + (t->char_num - t->char_freq_num) * (t->block_num - 1) * sizeof (unsigned long);

    //fseek(t->ifp, t->char_table_pos, SEEK_SET);
    fseek(t->ifp, p, SEEK_SET);

}

int _cmp_char_by_code(const void * c1, const void * c2) {
    // ascending order
    return (int) ((character *) c1)->c - ((character *) c2)->c;
}

int _cmp_char_by_freq(const void * c1, const void * c2) {
    // descending order
    return (int) ((*((character **) c2))->ss - (*((character **) c1))->ss);
}

/**
 * calculate how many chars can be stored in memory 
 * as an occ value in a occ snapshot.
 * return values:
 * =0         - memory isn't enough to contain any snapshots for any chars.
 * >=char_num - all occ snapshots can be stored in memory
 * in between - only the most freq chars
 */
unsigned long compute_mem_maxchars(bwttext * t) {

    unsigned long allowed, misc, snapshot;

    misc = 15;
    allowed = t->file_size / 2 + 2048 - t->char_num * sizeof (character) - misc;
    snapshot = sizeof (long) * t->char_num; //size for each occ snapshot

    //block number = snapshot number + 1
    t->block_num = allowed / snapshot + 1;
    //interval between snapshots
    t->block_width = t->file_size / t->block_num;
    if (t->file_size % t->block_num > 0) t->block_width++;

    //memory available for each snapshot / size of one occ value
    return OCCTABLE_MEMORY / t->block_num / sizeof (unsigned long);

}

void chartable_compute_charfreq(bwttext * t) {

    character * ch;
    character * chars[256];
    int c, i, f1, f2;
    unsigned long cc, max;

    t->char_num = 0;
    chartable_inithash(t);

    // count char freq

    cc = 0;
    fseek(t->fp, 4, SEEK_SET);
    while ((c = fgetc(t->fp)) != EOF) {
        ch = t->char_hash[c];
        if (ch == NULL) {
            // new char
            ch = t->char_hash[c] = &t->char_table[t->char_num++];
            ch->ss = 1; // freq=1
            ch->c = (unsigned char) c;
        } else {
            ch->ss++; // freq++
        }
        cc++;
    }

    t->file_size = cc;

    //max chars in memory as freq threshold

    max = compute_mem_maxchars(t);
    t->char_freq_num = max > t->char_num ? t->char_num : (unsigned short) max;

    //take the most freq ones

    for (i = 0; i < t->char_num; i++)
        chars[i] = &t->char_table[i];

    qsort(chars, t->char_num, sizeof (character *), _cmp_char_by_freq);

    f1 = f2 = 0;
    for (i = 0; i < t->char_num; i++) {
        chars[i]->isfreq = (unsigned char) (i < max);
        if (chars[i]->isfreq)
            chars[i]->i = f1++;
        else
            chars[i]->i = f2++;
    }

}

/**
 * calculate smaller symbols (ss) using freq 
 * to generate data for the C[] table
 */
void chartable_compute_ss(bwttext * t) {

    int c;
    unsigned long sbefore, tsbefore;
    character * ch;

    // sort characters lexicographically
    qsort(t->char_table, t->char_num, sizeof (character), _cmp_char_by_code);
    chartable_inithash(t); // set all null

    c = 0; // count for boudndary
    sbefore = 0;
    ch = t->char_table; // the smallest
    while (c++ < t->char_num) {
        // re-hash
        t->char_hash[(unsigned int) ch->c] = ch;
        // calculate smaller symbols
        tsbefore = sbefore;
        sbefore += ch->ss; // accumulate freq
        ch->ss = tsbefore; // smaller symbols
        ch++; // a larger char
    }

}

void chartable_save(bwttext * t) {

    chartable_init(t);

    fwrite(t->char_table, sizeof (character), t->char_num, t->ifp);

}

void chartable_load(bwttext * t) {
    character * ch;
    int i;

    chartable_init(t);

    fread(t->char_table, sizeof (character), t->char_num, t->ifp);

    // hash chars
    chartable_inithash(t);
    ch = t->char_table;
    for (i = 0; i < t->char_num; i++) {
        t->char_hash[ch->c] = ch;
        ch++;
    }

}

