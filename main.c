/*
 * csize.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "huffman.h"
#include "lzw.h"

int main(int argc, char **argv) {

    FILE * fp;

    fp = fopen("file1.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", huffman_csize(fp));
    fclose(fp);

    fp = fopen("file1.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 9));
    fclose(fp);

    fp = fopen("file1.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 20));
    fclose(fp);

    fp = fopen("file2.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", huffman_csize(fp));
    fclose(fp);

    fp = fopen("file2.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 9));
    fclose(fp);

    fp = fopen("file2.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 10));
    fclose(fp);

    fp = fopen("file3.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", huffman_csize(fp));
    fclose(fp);

    fp = fopen("file3.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 9));
    fclose(fp);

    fp = fopen("file3.txt", "rb");
    if (!fp) return 1;
    printf("%d\n", lzw_csize(fp, 15));
    fclose(fp);

/*
    FILE * fp = fopen("file1.txt", "r");
    FILE * fout = fopen("file1.lzw", "w");

    if (!fp) return 1;
    if (!fout) return 2;

    lzw_compress(fp, fout, 9);
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

    FILE * fp;

    if (argc == 3 && argv[1][0] == '-' && argv[1][1] == 'h') {

        fp = fopen(argv[2], "rb");
        if (!fp) return 1;
        printf("%d\n", huffman_csize(fp));
        fclose(fp);

    } else if (argc > 3 && argv[1][0] == '-' && argv[1][1] == 'l') {

        int w = atoi(argv[2]);
        if (w > 8 && w < 21) {
            // between 9 and 20

            fp = fopen(argv[3], "rb");
            if (!fp) return 1;
            printf("%d\n", lzw_csize(fp, w));
            fclose(fp);

        }

    }
    */

    return 0;
}
