
/**
 * bwtsearch: implements BWT backward search
 */

typedef struct {
    unsigned long first;
    unsigned long last;
} fpos_range;

/**
 * @param bwttext *         ref to processed bwttext
 * @param unsigned char *   ref to the queried pattern
 * @param unsigned int      length of the pattern
 * @return range in the first column of the matrix
 */
fpos_range * search_range(bwttext * t, unsigned char * p, unsigned int l);

unsigned long occ(bwttext * t, unsigned char c, unsigned long pos);


// TODO reverse data

