
#include "bwttext.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

void bwttext_read(bwttext * t, unsigned char special_char) {
    character * ch, * pch;
    unsigned short ch_repeats;
    unsigned long ss, tss;
    int ic;

    for (ic = 0; ic < 256; ic++)
        t->char_hash[ic] = NULL;
    t->char_num = 0;

    ss = 0;
    ch_repeats = 0;
    pch = NULL;
    while ((ic = fgetc(t->fp)) != EOF) {
        ch = t->char_hash[ic];
        if (ch == NULL) {
            ch = t->char_hash[ic] = &t->char_table[t->char_num++];
            ch->c = (unsigned char) ic;
            ch->ss = 1; // freq
        } else {
            ch->ss++;
        }
        if (t->rl_size < RL_MAXSIZE) {
            if (pch != ch || ch_repeats == USHRT_MAX) {
                t->rl_c[t->rl_size] = ch->c;
                t->rl_l[t->rl_size] = ch_repeats;
                t->rl_size++;
                ch_repeats = 1;
                pch = ch;
            } else {
                ch_repeats++;
            }
        }
        ss++;
    }
    if (t->rl_size < RL_MAXSIZE && pch != NULL && ch_repeats > 0) {
        t->rl_c[t->rl_size] = ch->c;
        t->rl_l[t->rl_size] = ch_repeats;
        t->rl_size++;
    }
    t->file_size = ss;

    ss = 0;
    ch = t->char_hash[special_char];
    if (ch != NULL) {
        ss = ch->ss;
        ch->ss = 0;
    }
    for (ic = 0; ic < 256; ic++) {
        ch = t->char_hash[ic];
        if (ic == special_char) continue;
        if (ch == NULL) continue;
        tss = ch->ss;
        ch->ss = ss;
        ss += tss;
    }

}

bwttext * bwttext_init(char * bwtfile, unsigned char special_char) {
    FILE * fp;

    fp = fopen(bwtfile, "rb");
    if (fp == NULL)
        exit(1);

    {
        bwttext * t = (bwttext *) malloc(sizeof (bwttext));

        t->fp = fp;

        // TODO seek if necessary
        fread(&t->end, sizeof (unsigned long), 1, t->fp);

        bwttext_read(t, special_char);

        return t;
    }
}

void bwttext_free(bwttext * t) {
    if (t != NULL) {
        if (t->fp != NULL) {
            fclose(t->fp);
            t->fp = NULL;
        }
        free(t);
    }
}

