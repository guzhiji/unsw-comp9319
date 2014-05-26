
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dump_occ(bwttext * t, unsigned char c, char * outfile) {
    FILE * ttt = fopen(outfile, "w");
    int ttc;
    unsigned long pos = 0;
    fpos_t fpos;
    fseek(t->fp, 4, SEEK_SET);
    while ((ttc = fgetc(t->fp)) != EOF) {
        fgetpos(t->fp, &fpos);
        fprintf(ttt, "%lu %lu\n", pos, occ(t, c, pos));
        fsetpos(t->fp, &fpos);
        pos++;
    }
    fclose(ttt);
}

void dump_occ_japan() {
    bwttext * t = bwttext_init("../tests/bwtsearch/japan.bwt", "../japan.idx", ' ', 1);
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

unsigned long lpos2(bwttext * t, unsigned char c, unsigned long occ) {
    int i, r;
    unsigned long n = 0, p = 0;
    unsigned char cblk[1024];

    fseek(t->fp, 4, SEEK_SET);

    do {
        r = fread(&cblk, sizeof (unsigned char), 1024, t->fp);
        for (i = 0; i < r; i++) {
            if (cblk[i] == c && n++ == occ)
                return p;
            p++;
        }
    } while (r > 0);
    return p;
}

void dump_lpos(bwttext * t, unsigned char c, FILE * fout) {
    int ic;
    unsigned long occ = 0, pos = 0, lp1, lp2;
    fpos_t fpos;
    fseek(t->fp, 4, SEEK_SET);
    fprintf(fout, "\n%d:\n", c);
    //    fprintf(fout, "%s\t%s\t%s\t%s\n", "occ", "pos", "lpos()", "lpos2()");
    fprintf(fout, "%s\t%s\t%s\n", "occ", "pos", "lpos()");
    while ((ic = fgetc(t->fp)) != EOF) {
        fgetpos(t->fp, &fpos);
        if (c == ic) {
            lp1 = lpos(t, c, occ);
            fprintf(fout, "%lu\t%lu\t%lu\n", occ, pos, lp1);
            //            fprintf(fout, "%lu\t%lu\t%lu\n", occ, pos, pos);
            //            lp2 = lpos2(t, c, occ);
            //            fprintf(fout, "%lu\t%lu\t%lu\t%lu\n", occ, pos, lp1, lp2);
            //            fprintf(fout, "%lu\t%lu\t%lu\t%lu\n", occ, pos, pos, pos);
            occ++;
        }
        fsetpos(t->fp, &fpos);
        pos++;
    }
}

void dump_lpos_japan() {
    bwttext * t = bwttext_init("../tests/bwtsearch/5MB.bwt", "../5MB.idx", ' ', 1);
    //    int i;

    //        dump_lpos(t, (unsigned char) 32, stdout);
    dump_lpos(t, 'P', stdout);
    //    for (i = 0; i < 256; i++) {
    //        dump_lpos(t, (unsigned char) i, stdout);
    //    }

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

    unsigned char special_char = '\n'; // TODO special char='['
    int special_char_post = 1; // TODO 0, before word

    if (argc > 3) {
        // 0: program name
        // 1: bwt file
        // 2: idx file
        // 3: ?

        char * opt_o = "-o";
        bwttext * t = bwttext_init(argv[1], argv[2], special_char, 0);

        if (argc > 4 && 0 == strcmp(argv[3], opt_o)) {// decoding
            // 3: "-o"
            // 4: unbwt file

            FILE * out = fopen(argv[4], "wb");
            decode_backward_rev(t, out);
            fclose(out);

        } else {// searching
            // 3: query term

            search(t, (unsigned char *) argv[3], strlen(argv[3]),
                    special_char, special_char_post);

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
    //    dump_lpos_japan();
    //    return 0;
}
