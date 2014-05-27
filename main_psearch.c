
#include "bwttext.h"
#include "occ.h"
#include "bwtsearch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

    unsigned char special_char = '['; // delimiter
    int special_char_post = 0; // delimiter before content, not after

    if (argc > 2) {
        // 0: program name
        // 1: bwt file

        bwttext * t = bwttext_init(argv[1], special_char);

        if (argc == 3) {// searching
            // 2: query term

            search(t, (unsigned char *) argv[2], strlen(argv[2]),
                    special_char, special_char_post);

        } else if (argc == 4) {// decoding a range
            // 2: starting record
            // 3: ending record

            decode_range(t, atol(argv[2]), atol(argv[3]),
                    special_char, special_char_post, stdout);

        }

        bwttext_free(t);

        return 0;

    }

    // error
    return 1;

}

