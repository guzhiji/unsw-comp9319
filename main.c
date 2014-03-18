/*
 * csize.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

//#include "huffman.h"
#include "lzw.h"

int main(int argc, char **argv)
{

/*
    FILE * fp = fopen("file3.txt", "rb");

    if (!fp) return 1;

    printf("%d", huffman_csize(fp));

    fclose(fp);
*/

    FILE * fp = fopen("file1.txt", "rb");

    if (!fp) return 1;

    printf("%d", lzw_csize(fp, 9));

    fclose(fp);
/*
    FILE * fp = fopen("data.txt", "r");
    FILE * fout = fopen("data.lzw", "w");

    if (!fp) return 1;
    if (!fout) return 2;

    lzw_compress(fp, fout, (unsigned short)12);
    //printf("%d", lzw_csize(fp, 12));

    fclose(fout);
    fclose(fp);
*/

    /*
    int c;
    while (c = fgetc(fp), c != EOF) printf("%d ", c);
    rewind(fp);
    while (c = _getsymbol(fp), c != EOF) printf("%d ", c);
    fclose(fp);
    */

    /*
    printf("\n");

    fp = fopen("data.txt", "r, ccs=UTF-8");
    if (!fp) return 1;

    wint_t wc;
    do {
        wc = fgetwc(fp);
        printf("wc=%04X ", wc);
    } while (wc != WEOF);
    fclose(fp);
    */

    return 0;
}
