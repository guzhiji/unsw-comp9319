
#include "bwttext.h"

#include "chartable.h"
#include "occtable.h"
#include <stdlib.h>
#include <stdio.h>

void bwttext_index_build(bwttext * t, unsigned char special_char) {

    chartable_compute_charfreq(t);

    occtable_init(t);
    occtable_generate(t); //requires freq

    chartable_compute_ss(t, special_char); //consumes freq

}

bwttext * bwttext_init(char * bwtfile, unsigned char special_char) {

    bwttext * t = (bwttext *) malloc(sizeof (bwttext));

    t->fp = fopen(bwtfile, "rb");
    if (t->fp == NULL) {
        bwttext_free(t);
        exit(1);
    }
    //fread(&t->end, sizeof (unsigned long), 1, t->fp);
    bwttext_index_build(t, special_char);

    return t;
}

void bwttext_free(bwttext * t) {
    if (t != NULL) {
        if (t->fp != NULL) {
            fclose(t->fp);
            t->fp = NULL;
        }
        occtable_free(t);
        free(t);
    }
}

