
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dump_occ(bwttext * t, unsigned char c, char * outfile) {
    FILE * ttt = fopen(outfile, "w");
    int ttc;
    unsigned long pos = 0;
    fseek(t->fp, 4, SEEK_SET);
    while ((ttc = fgetc(t->fp)) != EOF) {
        fprintf(ttt, "%lu %lu\n", pos, occ(t, c, pos));
        pos++;
    }
    fclose(ttt);
}

void dump_occ_japan() {
    bwttext * t = bwttext_init("../tests/bwtsearch/japan.bwt", "../japan.idx", 1);
    int i;
    char fn[50];
    /*
        dump_occ(t, (unsigned char) 166, "../japan.c.out");
     */

    for (i = 0; i < 256; i++) {
        sprintf(fn, "../occtest/japan.c.%d.out", i);
        dump_occ(t, (unsigned char) i, fn);
    }

    bwttext_free(t);
}

void dump_chartable(bwttext * t) {
    int i;
    character * ch;
    printf("==================\n");
    printf("dump_chartable:\n");
    for (i = 0; i < 256; i++) {
        ch = t->char_hash[i];
        if (ch == NULL) continue;
        printf("%c (%d): ss=%lu\n", ch->c, ch->c, ch->ss);
    }
    printf("==================\n");
}

int main(int argc, char **argv) {

    printf("sizeof character=%d\n", sizeof(character));
    printf("sizeof bwtblock=%d\n", sizeof(bwtblock));
    printf("sizeof bwtblock_index=%d\n", sizeof(bwtblock_index));

    if (argc > 3) {
        // 0: program name
        // 1: bwt file
        // 2: idx file
        // 3: ?

        char * opt_o = "-o";
        bwttext * t = bwttext_init(argv[1], argv[2], 0);
        printf("filesize=%lu\n", t->file_size);

        if (argc > 4 && 0 == strcmp(argv[3], opt_o)) {// decoding
            // 3: "-o"
            // 4: unbwt file

            FILE * out = fopen(argv[4], "wb");
            decode_backward_rev(t, out);
            fclose(out);

        } else {// searching
            // 3: query term

            search(t, (unsigned char *) argv[3], strlen(argv[3]));

        }

        bwttext_free(t);

        return 0;

    }

    // error
    return 1;

    //--------------------------------------------------------------------------
    //    bwttext * t;
    //    t = bwttext_init("../tests/bwtsearch/tiny.bwt", "../tiny.idx", 1);
    //    //            t = bwttext_init("../tests/bwtsearch/japan.bwt", "../japan.idx", 1);
    //    //            t = bwttext_init("../tests/bwtsearch/sherlock.bwt", "../sherlock.idx", 1);
    //    //            t = bwttext_init("../tests/bwtsearch/pride.bwt", "../pride.idx", 1);
    //    //            t = bwttext_init("../tests/bwtsearch/gcc.bwt", "../gcc.idx", 1);
    //
    //    dump_chartable(t);
    //
    //    //            decode_backward(t, stdout);
    //
    //    bwttext_free(t);
    //    return 0;
    //--------------------------------------------------------------------------
    //    dump_occ_japan();
    //    return 0;
}
