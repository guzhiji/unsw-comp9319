
#include <stdio.h>
#include <stdlib.h>

#include "bwtencoder.h"

int main(int argc, char ** argv) {
    if (argc == 3) {

        FILE * in, * out;

        in = fopen(argv[1], "r");
        if (in == NULL) return 1;
        out = fopen(argv[2], "w");
        if (out == NULL) {
            fclose(in);
            return 1;
        }

        //pbwt(in, out, '[', 1, 1);
        pbwt(in, out, '\n', 1, 1);

        fclose(out);
        fclose(in);
        return 0;
    }
    return 1;
}

