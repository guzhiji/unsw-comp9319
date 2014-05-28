
#include "bwttext.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

    unsigned char special_char = '\n'; // delimiter
    int special_char_post = 1; // delimiter after content

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
        } else if (argc == 4) {// searching
            // 3: query term

            search(t, (unsigned char *) argv[3], strlen(argv[3]),
                    special_char, special_char_post);

        } else if (argc == 5) {// decoding a range
            // 3: starting record
            // 4: ending record

            decode_range(t, atol(argv[3]), atol(argv[4]),
                    special_char, special_char_post, stdout);

        }

        bwttext_free(t);

        return 0;

    }

    // error
    return 1;

}

