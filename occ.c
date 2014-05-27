
#include "occ.h"

#include <stdio.h>
#include <stdlib.h>

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos) {
    unsigned long o, l, i;
    o = 0;
    l = 0;
    for (i = 0; i < t->rl_size; i++) {
        if (pos < t->rl_l[i] + l) {
            if (t->rl_c[i] == c)
                return pos - l + o;
            return o;
        }
        l += t->rl_l[i];
        if (t->rl_c[i] == c)
            o += t->rl_l[i];
    }

    fseek(t->fp, l, SEEK_SET);
    {
        unsigned char buf[1024];
        int r;
        do {
            r = fread(buf, sizeof(unsigned char), 1024, t->fp);
            for (i = 0; i < r; i++) {
                if (pos == l++) return o;
                if (buf[i] == c) o++;
            }
        } while (r > 0);
    }
    return o;
}

/**
 * find the first position as the given occurance occ of c 
 * in BWT text (pos - position),
 * the last column in the rotation matrix of the 
 * original text (l - last column).
 */
unsigned long lpos(bwttext * t, unsigned char c, unsigned long occ) {
    unsigned long o, l, i;
    o = 0;
    l = 0;
    for (i = 0; i < t->rl_size; i++) {

        if (t->rl_c[i] == c 
                && occ < t->rl_l[i] + o)
            return occ - o + l;

        l += t->rl_l[i];
        if (t->rl_c[i] == c)
            o += t->rl_l[i];
    }

    fseek(t->fp, l, SEEK_SET);
    {
        unsigned char buf[1024];
        int r;

        do {
            r = fread(buf, sizeof (unsigned char), 1024, t->fp);
            for (i = 0; i < r; i++) {
                // when c occurs, o is compared against occ before it counts;
                if (buf[i] == c && o++ == occ)
                    return l; // l is returned before it counts the current position
                l++;
            }
        } while (r > 0);

    }
    return l;
}

