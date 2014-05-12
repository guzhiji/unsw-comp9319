
#include "bwtblock.h"

/**
 * @param t     BWT text object
 * @param pos   position in BWT text
 * @return      char offset of the block where the position locates
 */
unsigned long bwtblock_offset(bwttext * t, unsigned long pos) {
    // number of blocks * number of chars per block - 1 + 1
    return pos / t->block_width * t->block_width;
}

/**
 * look up a block through occ snapshots
 *
 * @param s     snapshots (occ values)
 * @param l     number of snapshots
 * @param occ   the target occ value
 * @return      block id (0-based)
 */
unsigned int _bwtblock_lookup(unsigned long * s, unsigned int l, unsigned long occ) {
    unsigned int i;

    // if it isn't after snapshot i,
    // it is snapshot i-1;
    // the block after snapshot i-1 is block i;
    // even for the one before snapshot 0,
    // it is block 0
    for (i = 0; i < l; i++) {
        if (occ < s[i])
            return i;
        // A problem arises because of change of block width 
        // after finding that file size isn't divideable by 
        // block number in compute_mem_maxchars() of chartable.c.
        // Consequently, there can be some slots not being used here,
        // so more code is added to detect the issue.
        if (i > 0 && s[i - 1] > 0 && s[i] == 0)
            return i - 1;
    }
    // after all snapshots,
    // or after the last snapshot, snapshot l-1,
    // it is block l (since blocks=snapshots+1)
    return l;

}

void bwtblock_start_lookup(bwttext * t, character * ch, unsigned long occ, bwtblock_start * start) {
    unsigned long l, offset, blk;

    l = t->block_num - 1; // snapshot count
    offset = ch->i * l; // start position of the char

    if (ch->isfreq) {

        blk = _bwtblock_lookup(&t->occ_freq[offset], l, occ);
        if (blk > 0) // for block 0, occ starts from 0, not stored
            start->occ = t->occ_freq[offset + blk - 1]; // char start + snapshot index

    } else {
        unsigned long buf[1024];
        unsigned int bl, rl, i; //buf len, remaining len
        // same logic

        fseek(t->ifp, t->occ_infreq_pos + offset * sizeof (unsigned long), SEEK_SET);

        // _blkoffset_lookup() by buf
        blk = 0;
        rl = l;
        do {

            if (rl > 1024)
                rl -= bl = 1024;
            else
                rl -= bl = rl;

            fread(buf, sizeof (unsigned long), bl, t->ifp);
            blk += i = _bwtblock_lookup(buf, bl, occ);
            if (i < bl) // before the last snapshot within the current buf
                break;

        } while (rl > 0);

        if (blk > 0) {

            fseek(t->ifp, t->occ_infreq_pos + (offset + blk - 1) * sizeof (unsigned long), SEEK_SET);
            fread(&start->occ, sizeof (unsigned long), 1, t->ifp);

        }

    }

    if (blk == 0)
        start->occ = 0;
    start->offset = blk * t->block_width;

}

/**
 * count occ in a block.
 */
unsigned long bwtblock_occ(bwttext * t, bwtblock_start * start, unsigned char c, unsigned long pos_until) {

    unsigned char buf[1024];
    int ic, r;
    unsigned long pos, o;

    pos = start->offset;
    o = start->occ;
    fseek(t->fp, 4 + pos, SEEK_SET);
    do {
        r = fread(buf, sizeof (unsigned char), 1024, t->fp);
        for (ic = 0; ic < r; ic++) {
            if (pos_until == pos++) return o;
            if (buf[ic] == c) o++;
        }
    } while (r > 0);
    return o;

}

unsigned long bwtblock_occ_position(bwttext * t, bwtblock_start * start, unsigned char c, unsigned long occ_until) {
    unsigned long p, n;

    p = start->offset; // char position
    n = start->occ; // number of occurances

    // count occ until the given occ
    // (the position should be found within the block)
    fseek(t->fp, 4 + p, SEEK_SET);
    {
        unsigned char cblk[1024];
        int r, i;

        do {
            r = fread(cblk, sizeof (unsigned char), 1024, t->fp);
            for (i = 0; i < r; i++) {
                // when c occurs, n is compared against occ before it counts;
                if (cblk[i] == c && n++ == occ_until)
                    return p; // p is returned before it counts the current position
                p++;
            }
        } while (r > 0);

    }
    return p;
}
